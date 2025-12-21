document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
});

let isConfigured = false;

async function loadStatus() {
    try {
        const res = await fetch('/api/status');
        const data = await res.json();
        
        isConfigured = data.configured;
        
        const statusDiv = document.getElementById('status-content');
        statusDiv.innerHTML = `
            <p><strong>IP:</strong> ${data.ip}</p>
            <p><strong>Mode:</strong> ${data.state === 3 ? 'Access Point (Setup)' : 'Verbunden'}</p>
            <p><strong>Heap:</strong> ${Math.round(data.heap/1024)} KB</p>
        `;

        // Logik:
        // Wenn wir im AP Mode sind (State 3) -> Nur WLAN Config zeigen
        // Wenn wir verbunden sind -> Alles zeigen
        
        if (data.state !== 3) {
            document.getElementById('app-config-section').style.display = 'block';
            document.getElementById('wifi-section').style.display = 'none'; // Optional: Wifi ausblenden wenn verbunden? Oder lassen zum Ändern?
            // Lassen wir es da, aber vielleicht zugeklappt. Für jetzt: Einfach alles anzeigen wenn verbunden.
            document.getElementById('wifi-section').style.display = 'block';
        } else {
            // Setup Mode
            document.getElementById('app-config-section').style.display = 'none';
        }

    } catch (e) {
        console.error('Status Error', e);
    }
}

async function scanWifi() {
    const list = document.getElementById('wifi-list');
    list.innerHTML = 'Suche WLANs... (bitte warten)';
    
    try {
        const res = await fetch('/api/scan');
        const data = await res.json();
        
        list.innerHTML = '';
        data.networks.forEach(net => {
            const div = document.createElement('div');
            div.className = 'wifi-item';
            div.innerHTML = `<span>${net.ssid}</span> <span>${net.rssi} dBm ${net.secure ? '🔒' : '🔓'}</span>`;
            div.onclick = () => {
                document.getElementById('ssid').value = net.ssid;
                document.getElementById('password').focus();
            };
            list.appendChild(div);
        });
    } catch (e) {
        list.innerHTML = 'Fehler beim Scannen.';
        console.error(e);
    }
}

async function saveConfig() {
    const data = {
        ssid: document.getElementById('ssid').value,
        password: document.getElementById('password').value,
        apikey: document.getElementById('apikey').value,
        station: {
            name: document.getElementById('st_name').value,
            id: document.getElementById('st_id').value
        },
        line1: {
            name: document.getElementById('l1_name').value,
            dir: document.getElementById('l1_dir').value
        },
        line2: {
            name: document.getElementById('l2_name').value,
            dir: document.getElementById('l2_dir').value
        }
    };
    
    const btn = document.getElementById('save-btn');
    btn.disabled = true;
    btn.textContent = 'Speichere...';
    
    try {
        const res = await fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        
        const result = await res.json();
        alert(result.message);
    } catch (e) {
        alert('Fehler beim Speichern!');
        btn.disabled = false;
        btn.textContent = 'Speichern & Neustart';
    }
}

async function factoryReset() {
    if (!confirm("Wirklich alles löschen und zurücksetzen?")) return;
    
    try {
        const res = await fetch('/api/reset', { method: 'POST' });
        const result = await res.json();
        alert(result.message);
    } catch (e) {
        alert('Fehler beim Reset!');
    }
}
