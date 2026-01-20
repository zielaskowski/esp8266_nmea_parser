
const gps = new GPS();

// simple browser-side cache
const rawLines = [];
const MAX_LINES = 200;

gps.on('data', function(data) {
    if (data.type === 'GGA') {
        document.getElementById('lat').textContent =
            data.lat !== null ? data.lat.toFixed(6) : '–';

        document.getElementById('lon').textContent =
            data.lon !== null ? data.lon.toFixed(6) : '–';

        document.getElementById('fix').textContent =
            data.fix !== null ? data.fix : '–';

        document.getElementById('sats').textContent =
            data.satellites !== null ? data.satellites : '–';
    }
});

async function pollNMEA() {
    try {
        const r = await fetch('/nmea', { cache: 'no-store' });
        if (!r.ok) return;

        const text = await r.text();
        const lines = text.trim().split('\n');

        for (const line of lines) {
            if (!line.startsWith('$')) continue;

            rawLines.push(line);
            gps.update(line);

            if (rawLines.length > MAX_LINES) {
                rawLines.shift();
            }
        }

        const rawDiv = document.getElementById('raw');
        rawDiv.textContent = rawLines.join('\n');
        rawDiv.scrollTop = rawDiv.scrollHeight;

    } catch (e) {
        console.log('NMEA fetch failed');
    }
}

// poll every second
setInterval(pollNMEA, 1000);
