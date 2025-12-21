document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
    
    document.getElementById('config-form').addEventListener('submit', saveConfig);
});

async function loadStatus() {
    try {
        const res = await fetch('/api/status');
        const data = await res.json();
        
        const statusDiv = document.getElementById('status-content');
        statusDiv.innerHTML = `
            <p><strong>IP:</strong> ${data.ip}</p>
            <p><strong>Heap:</strong> ${Math.round(data.heap/1024)} KB</p>
            <p><strong>Konfiguriert:</strong> ${data.configured ? 'Ja' : 'Nein'}</p>
        `;
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

async function saveConfig(e) {
    e.preventDefault();
    
    const formData = new FormData(e.target);
    const data = {
        ssid: formData.get('ssid'),
        password: formData.get('password'),
        apikey: formData.get('apikey'),
        station: {
            name: formData.get('station_name'),
            id: formData.get('station_id')
        },
        line1: {
            name: formData.get('l1_name'),
            dir: formData.get('l1_dir')
        },
        line2: {
            name: formData.get('l2_name'),
            dir: formData.get('l2_dir')
        }
    };
    
    const btn = e.target.querySelector('button[type="submit"]');
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

