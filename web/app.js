const gps = new GPS();

// simple browser-side cache
const rawLines = [];
const MAX_LINES = 100;
// GSVvstate={
// 	"GPS":{"
// 		"GPS24":28,    key:snr
//      	}
//	}
const GSVstate = {};
// charts={
// 	"GPS": chart_obj
// }
const charts = {};

function createChart(canvasId) {
  // make sure DOMContentLoaded
  ctx = document.getElementById(canvasId).getContext("2d");

  return new Chart(ctx, {
    type: "bar",
    data: {
      labels: [],
      datasets: [
        {
          label: "GPS SNR (dB-Hz)",
          data: [],
        },
      ],
    },
    options: {
      animation: false,
      responsive: true,
      scales: {
        y: {
          min: 0,
          max: 60,
          title: { display: true, text: "SNR" },
        },
        x: {
          title: { display: true, text: "Satelites" },
        },
      },
    },
  });
}

function updateGPSChart() {
  for (const system in GSVstate) {
    const entries = Object.entries(GSVstate[system]).sort((a, b) =>
      a[0].localeCompare(b[0]),
    );
    charts[system].data.labels = entries.map((e) => e[0]);
    charts[system].data.datasets[0].data = entries.map((e) => e[1]);
    charts[system].update();
  }
}

function createCanvas(system) {
  // wrapper (title + canvas)
  const container = document.getElementById("charts");

  const box = document.createElement("div");
  box.style.marginBottom = "24px";

  const title = document.createElement("h2");
  title.textContent = system;
  box.appendChild(title);

  const canvas = document.createElement("canvas");
  canvas.id = system;
  canvas.height = 200;
  canvas.width = 400;
  box.appendChild(canvas);

  container.appendChild(box);
}

const addGSVPage = (el) => {
  //add system if missing
  if (GSVstate[el.system] === undefined) {
    GSVstate[el.system] = {};
    createCanvas(el.system);
    charts[el.system] = createChart(el.system);
  }
  // add satelite key if missing
  if (GSVstate[el.system][el.key] === undefined) {
    GSVstate[el.system][el.key] = 0;
  }
  if (!el.snr) return; // no new data or zero

  GSVstate[el.system][el.key] = el.snr;
};

gps.on("data", function (data) {
  //GSV list of satelites in view
  if (data.type === "GSV") {
    data.satellites.forEach(addGSVPage);
  }

  //GGA fix information
  if ((data.type === "GGA") & data.valid) {
    document.getElementById("time").textContent = data.time;
    document.getElementById("lat").textContent =
      data.lat !== null ? data.lat.toFixed(6) : "–";
    document.getElementById("lon").textContent =
      data.lon !== null ? data.lon.toFixed(6) : "–";
    document.getElementById("fix").textContent =
      data.fix !== null ? data.fix : "–";
    // horizontal dilution of precision
    hdopBlock: if (data.hdop !== null) {
      document.getElementById("hdop").textContent = data.hdop;
      // mnemonic description of hdop (see https://en.wikipedia.org/wiki/Dilution_of_precision)
      if (data.hdop < 1) {
        document.getElementById("hdop_mnemonic").textContent = "ideal";
        break hdopBlock;
      }
      if (data.hdop < 2) {
        document.getElementById("hdop_mnemonic").textContent = "excelent";
        break hdopBlock;
      }
      if (data.hdop < 5) {
        document.getElementById("hdop_mnemonic").textContent = "good";
        break hdopBlock;
      }
      if (data.hdop < 10) {
        document.getElementById("hdop_mnemonic").textContent = "moderate";
        break hdopBlock;
      }
      if (data.hdop < 20) {
        document.getElementById("hdop_mnemonic").textContent = "fair";
        break hdopBlock;
      }
      document.getElementById("hdop_mnemonic").textContent = "poor";
      break hdopBlock;
    } else {
      document.getElementById("hdop").textContent = "-";
    }

    document.getElementById("sats").textContent =
      data.satellites !== null ? data.satellites : "–";
  }
});

async function pollNMEA() {
  try {
    const resp = await fetch("/nmea", { cache: "no-store" });
    if (!resp.ok) return;

    const text = await r.text();
    const lines = text.trim().split("\n");

    for (const line of lines) {
      if (!line.startsWith("$")) continue;

      rawLines.push(line);
      gps.update(line);

      if (rawLines.length > MAX_LINES) {
        rawLines.shift();
      }
    }

    const rawDiv = document.getElementById("raw");
    rawDiv.textContent = rawLines.join("\n");
    rawDiv.scrollTop = rawDiv.scrollHeight;
  } catch (e) {
    console.log("NMEA fetch failed");
  }
}

window.addEventListener("DOMContentLoaded", () => {
  // poll every second
  setInterval(pollNMEA, 1e3);
  // update every 5 second
  setInterval(updateGPSChart, 5000);
});
