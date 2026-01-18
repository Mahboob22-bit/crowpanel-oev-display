document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
    setupStopSearch();
});

let isConfigured = false;

// =====================
// Debounce Utility
// =====================
function debounce(fn, delay) {
    let timeoutId;
    return function(...args) {
        clearTimeout(timeoutId);
        timeoutId = setTimeout(() => fn.apply(this, args), delay);
    };
}

// =====================
// Stop Search Functions
// =====================
function setupStopSearch() {
    const searchInput = document.getElementById('stop-search');
    if (!searchInput) return;
    
    const debouncedSearch = debounce(async (query) => {
        if (query.length < 2) {
            hideStopResults();
            return;
        }
        await searchStops(query);
    }, 300);
    
    searchInput.addEventListener('input', (e) => {
        debouncedSearch(e.target.value.trim());
    });
    
    // Hide dropdown when clicking outside
    document.addEventListener('click', (e) => {
        const wrapper = document.querySelector('.search-wrapper');
        if (wrapper && !wrapper.contains(e.target)) {
            hideStopResults();
        }
    });
}

async function searchStops(query) {
    const resultsDiv = document.getElementById('stop-results');
    resultsDiv.innerHTML = '<div class="dropdown-item loading">Suche...</div>';
    resultsDiv.style.display = 'block';
    
    try {
        const res = await fetch(`/api/stops/search?q=${encodeURIComponent(query)}`);
        const data = await res.json();
        
        if (data.error) {
            resultsDiv.innerHTML = `<div class="dropdown-item error">${data.error}</div>`;
            return;
        }
        
        renderStopResults(data.results || []);
    } catch (e) {
        console.error('Stop search error:', e);
        resultsDiv.innerHTML = '<div class="dropdown-item error">Fehler bei der Suche</div>';
    }
}

function renderStopResults(results) {
    const resultsDiv = document.getElementById('stop-results');
    
    if (!results || results.length === 0) {
        resultsDiv.innerHTML = '<div class="dropdown-item">Keine Ergebnisse</div>';
        return;
    }
    
    resultsDiv.innerHTML = '';
    results.forEach(stop => {
        const div = document.createElement('div');
        div.className = 'dropdown-item';
        div.innerHTML = `<strong>${stop.name}</strong><span class="location">${stop.location || ''}</span>`;
        div.onclick = () => selectStop(stop.id, stop.name);
        resultsDiv.appendChild(div);
    });
}

function selectStop(id, name) {
    document.getElementById('st_id').value = id;
    document.getElementById('st_name').value = name;
    
    const selectedDiv = document.getElementById('selected-stop');
    selectedDiv.innerHTML = `<strong>${name}</strong> <span class="stop-id">(${id})</span>`;
    selectedDiv.classList.add('has-selection');
    
    hideStopResults();
    document.getElementById('stop-search').value = '';
}

function hideStopResults() {
    const resultsDiv = document.getElementById('stop-results');
    if (resultsDiv) {
        resultsDiv.style.display = 'none';
        resultsDiv.innerHTML = '';
    }
}

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
            
            // Load current config into form fields
            loadCurrentConfig(data);
        } else {
            // Setup Mode
            document.getElementById('app-config-section').style.display = 'none';
        }

    } catch (e) {
        console.error('Status Error', e);
    }
}

function loadCurrentConfig(data) {
    // Station
    if (data.station && data.station.id) {
        document.getElementById('st_id').value = data.station.id;
        document.getElementById('st_name').value = data.station.name;
        
        const selectedDiv = document.getElementById('selected-stop');
        selectedDiv.innerHTML = `<strong>${data.station.name}</strong> <span class="stop-id">(${data.station.id})</span>`;
        selectedDiv.classList.add('has-selection');
    }
    
    // Line 1
    if (data.line1) {
        document.getElementById('l1_name').value = data.line1.name || '';
        document.getElementById('l1_dir').value = data.line1.dir || '';
    }
    
    // Line 2
    if (data.line2) {
        document.getElementById('l2_name').value = data.line2.name || '';
        document.getElementById('l2_dir').value = data.line2.dir || '';
    }
}

async function scanWifi() {
    const list = document.getElementById('wifi-list');
    list.innerHTML = 'Suche WLANs... (bitte warten)';
    
    try {
        // 1. Scan starten
        const startRes = await fetch('/api/scan');
        const startData = await startRes.json();
        
        if (startData.status !== 'started' && startData.status !== 'running') {
            throw new Error(startData.message || 'Start failed');
        }

        // 2. Pollen auf Ergebnisse
        let attempts = 0;
        const maxAttempts = 20; // 20 * 500ms = 10 Sekunden Timeout
        
        const poll = async () => {
            if (attempts >= maxAttempts) {
                list.innerHTML = 'Timeout beim Scannen.';
                return;
            }
            
            attempts++;
            const res = await fetch('/api/scan-results');
            const data = await res.json();
            
            if (data.status === 'running') {
                list.innerHTML = `Suche läuft... (${attempts})`;
                setTimeout(poll, 500);
            } else if (data.status === 'complete') {
                // Ergebnisse anzeigen
                renderNetworks(data.networks);
            } else {
                list.innerHTML = 'Fehler: ' + (data.message || 'Unbekannt');
            }
        };
        
        // Start polling
        setTimeout(poll, 500);
        
    } catch (e) {
        list.innerHTML = 'Fehler beim Scannen.';
        console.error(e);
    }
}

function renderNetworks(networks) {
    const list = document.getElementById('wifi-list');
    list.innerHTML = '';
    
    if (!networks || networks.length === 0) {
        list.innerHTML = 'Keine Netzwerke gefunden.';
        return;
    }

    networks.forEach(net => {
        const div = document.createElement('div');
        div.className = 'wifi-item';
        div.innerHTML = `<span>${net.ssid}</span> <span>${net.rssi} dBm ${net.secure ? '🔒' : '🔓'}</span>`;
        div.onclick = () => {
            document.getElementById('ssid').value = net.ssid;
            document.getElementById('password').focus();
        };
        list.appendChild(div);
    });
}

async function saveConfig() {
    const data = {
        ssid: document.getElementById('ssid').value,
        password: document.getElementById('password').value,
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
