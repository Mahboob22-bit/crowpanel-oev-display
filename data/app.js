document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
    setupStopSearch();
    loadFavorites();
});

let isConfigured = false;
let availableLines = [];
let currentStopId = null;
let refreshInterval = null;

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
let highlightedIndex = -1;

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
    
    // Keyboard navigation
    searchInput.addEventListener('keydown', (e) => {
        const resultsDiv = document.getElementById('stop-results');
        if (resultsDiv.style.display === 'none') return;
        
        const items = resultsDiv.querySelectorAll('.dropdown-item:not(.loading):not(.error)');
        if (items.length === 0) return;
        
        if (e.key === 'ArrowDown') {
            e.preventDefault();
            highlightedIndex = (highlightedIndex + 1) % items.length;
            updateHighlight(items);
        } else if (e.key === 'ArrowUp') {
            e.preventDefault();
            highlightedIndex = highlightedIndex <= 0 ? items.length - 1 : highlightedIndex - 1;
            updateHighlight(items);
        } else if (e.key === 'Enter') {
            e.preventDefault();
            if (highlightedIndex >= 0 && highlightedIndex < items.length) {
                items[highlightedIndex].click();
            }
        } else if (e.key === 'Escape') {
            e.preventDefault();
            hideStopResults();
        }
    });
    
    // Hide dropdown when clicking outside
    document.addEventListener('click', (e) => {
        const wrapper = document.querySelector('.search-wrapper');
        if (wrapper && !wrapper.contains(e.target)) {
            hideStopResults();
        }
    });
}

function updateHighlight(items) {
    items.forEach((item, index) => {
        if (index === highlightedIndex) {
            item.classList.add('highlighted');
            item.scrollIntoView({ block: 'nearest' });
        } else {
            item.classList.remove('highlighted');
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
    highlightedIndex = -1; // Reset highlight
    
    if (!results || results.length === 0) {
        resultsDiv.innerHTML = '<div class="dropdown-item">Keine Ergebnisse</div>';
        return;
    }
    
    resultsDiv.innerHTML = '';
    results.forEach(stop => {
        const div = document.createElement('div');
        div.className = 'dropdown-item';
        // Nur Stationsname anzeigen, ohne Ortsnamen
        div.innerHTML = `<strong>${stop.name}</strong>`;
        div.onclick = () => selectStop(stop.id, stop.name);
        resultsDiv.appendChild(div);
    });
}

async function selectStop(id, name) {
    document.getElementById('st_id').value = id;
    document.getElementById('st_name').value = name;
    currentStopId = id;
    
    const selectedDiv = document.getElementById('selected-stop');
    selectedDiv.innerHTML = `<strong>${name}</strong> <span class="stop-id">(${id})</span>`;
    selectedDiv.classList.add('has-selection');
    
    hideStopResults();
    document.getElementById('stop-search').value = '';
    
    // Favoriten speichern
    saveFavoriteStop(id, name);
    
    // Linien laden
    await loadLinesForStop(id);
}

function hideStopResults() {
    const resultsDiv = document.getElementById('stop-results');
    if (resultsDiv) {
        resultsDiv.style.display = 'none';
        resultsDiv.innerHTML = '';
    }
}

// =====================
// Line Loading Functions
// =====================
async function loadLinesForStop(stopId) {
    const linesSection = document.getElementById('lines-section');
    const loadingDiv = document.getElementById('line1-loading');
    const l1Select = document.getElementById('l1_select');
    const l2Select = document.getElementById('l2_select');
    
    linesSection.style.display = 'block';
    loadingDiv.style.display = 'block';
    l1Select.style.display = 'none';
    l2Select.style.display = 'none';
    
    try {
        const res = await fetch(`/api/lines?stopId=${encodeURIComponent(stopId)}`);
        const data = await res.json();
        
        if (data.error) {
            showToast('Fehler beim Laden der Linien: ' + data.error, 'error');
            loadingDiv.style.display = 'none';
            return;
        }
        
        availableLines = data.lines || [];
        renderLineDropdowns(availableLines);
        loadingDiv.style.display = 'none';
        l1Select.style.display = 'block';
        l2Select.style.display = 'block';
    } catch (e) {
        console.error('Line loading error:', e);
        showToast('Fehler beim Laden der Linien', 'error');
        loadingDiv.style.display = 'none';
    }
}

function renderLineDropdowns(lines) {
    const l1Select = document.getElementById('l1_select');
    const l2Select = document.getElementById('l2_select');
    
    // Linien nach Typ gruppieren und sortieren
    const typeOrder = { 'tram': 0, 'bus': 1, 'train': 2, 'metro': 3 };
    const groupedLines = {};
    
    lines.forEach(line => {
        const type = line.type || 'other';
        if (!groupedLines[type]) {
            groupedLines[type] = [];
        }
        groupedLines[type].push(line);
    });
    
    // Dropdowns leeren
    l1Select.innerHTML = '<option value="">Bitte wählen...</option>';
    l2Select.innerHTML = '<option value="">Bitte wählen...</option>';
    
    // Typen sortiert durchgehen
    const sortedTypes = Object.keys(groupedLines).sort((a, b) => {
        const orderA = typeOrder[a] !== undefined ? typeOrder[a] : 999;
        const orderB = typeOrder[b] !== undefined ? typeOrder[b] : 999;
        return orderA - orderB;
    });
    
    sortedTypes.forEach(type => {
        const typeLabel = getTypeLabel(type);
        
        // Optgroup für jeden Typ
        const optgroup1 = document.createElement('optgroup');
        optgroup1.label = typeLabel;
        
        const optgroup2 = document.createElement('optgroup');
        optgroup2.label = typeLabel;
        
        groupedLines[type].forEach(line => {
            const icon = getVehicleIcon(type);
            const optionText = `${icon} ${line.line} → ${line.dir}`;
            
            const option1 = document.createElement('option');
            option1.value = JSON.stringify({ line: line.line, dir: line.dir, type: type });
            option1.textContent = optionText;
            
            const option2 = document.createElement('option');
            option2.value = JSON.stringify({ line: line.line, dir: line.dir, type: type });
            option2.textContent = optionText;
            
            optgroup1.appendChild(option1);
            optgroup2.appendChild(option2);
        });
        
        l1Select.appendChild(optgroup1);
        l2Select.appendChild(optgroup2);
    });
    
    // Event Listener für Auswahl
    l1Select.onchange = () => handleLineSelection(1);
    l2Select.onchange = () => handleLineSelection(2);
}

function handleLineSelection(lineNumber) {
    const selectId = `l${lineNumber}_select`;
    const select = document.getElementById(selectId);
    
    if (select.value) {
        const lineData = JSON.parse(select.value);
        document.getElementById(`l${lineNumber}_name`).value = lineData.line;
        document.getElementById(`l${lineNumber}_dir`).value = lineData.dir;
        
        // Optional: Preview aktualisieren
        if (currentStopId) {
            loadDeparturePreview(currentStopId, lineData.line, lineData.dir);
        }
    }
}

function getVehicleIcon(type) {
    const icons = {
        'tram': '🚋',
        'bus': '🚌',
        'train': '🚆',
        'metro': '🚊',
        'rail': '🚄',
        'cableway': '🚡'
    };
    return icons[type] || '🚍';
}

function getTypeLabel(type) {
    const labels = {
        'tram': 'Tram',
        'bus': 'Bus',
        'train': 'Zug',
        'metro': 'Metro',
        'rail': 'Bahn',
        'cableway': 'Seilbahn'
    };
    return labels[type] || 'Andere';
}

// =====================
// Favorites Functions
// =====================
function saveFavoriteStop(stopId, stopName) {
    const MAX_FAVORITES = 5;
    let favorites = getFavorites();
    
    // Entferne existierendes Element (für LRU)
    favorites = favorites.filter(f => f.id !== stopId);
    
    // Füge an den Anfang hinzu
    favorites.unshift({ id: stopId, name: stopName });
    
    // Begrenze auf MAX_FAVORITES
    if (favorites.length > MAX_FAVORITES) {
        favorites = favorites.slice(0, MAX_FAVORITES);
    }
    
    localStorage.setItem('favoriteStops', JSON.stringify(favorites));
    renderFavorites();
}

function getFavorites() {
    try {
        const stored = localStorage.getItem('favoriteStops');
        return stored ? JSON.parse(stored) : [];
    } catch (e) {
        return [];
    }
}

function loadFavorites() {
    renderFavorites();
}

function renderFavorites() {
    const favoritesDiv = document.getElementById('favorite-stops');
    if (!favoritesDiv) return;
    
    const favorites = getFavorites();
    
    if (favorites.length === 0) {
        favoritesDiv.innerHTML = '';
        return;
    }
    
    favoritesDiv.innerHTML = '';
    favorites.forEach(fav => {
        const chip = document.createElement('div');
        chip.className = 'favorite-chip';
        chip.textContent = fav.name;
        chip.onclick = () => selectStop(fav.id, fav.name);
        favoritesDiv.appendChild(chip);
    });
}

// =====================
// Departure Preview Functions
// =====================
async function loadDeparturePreview(stopId, line, direction) {
    const previewDiv = document.getElementById('departure-preview');
    if (!previewDiv) return;
    
    // Suche passende Departures in den bereits geladenen Linien
    // Da wir die Abfahrten bereits beim Laden der Linien haben,
    // müssten wir diese zwischenspeichern oder nochmal abrufen
    
    // Für jetzt: Einfache Implementierung die zeigt dass die Linie ausgewählt wurde
    previewDiv.innerHTML = `
        <div class="preview-title">Ausgewählte Linie</div>
        <div class="preview-departure">
            ${getVehicleIcon(getLineType(line, direction))} 
            <strong>${line}</strong> → ${direction}
        </div>
        <div style="margin-top: 10px; font-size: 0.9em; color: #666;">
            Die aktuellen Abfahrtszeiten werden auf dem Display angezeigt.
        </div>
    `;
    previewDiv.classList.add('show');
}

function getLineType(line, direction) {
    // Versuche den Typ aus den verfügbaren Linien zu ermitteln
    for (const l of availableLines) {
        if (l.line === line && l.direction === direction) {
            return l.type;
        }
    }
    return 'other';
}

// =====================
// Live Departures Display
// =====================
async function loadLiveDepartures() {
    const contentDiv = document.getElementById('live-departures-content');
    
    try {
        // Hole die aktuelle Config
        const statusRes = await fetch('/api/status');
        const config = await statusRes.json();
        
        if (!config.station || !config.station.id) {
            contentDiv.innerHTML = '<div class="no-data">Keine Haltestelle konfiguriert</div>';
            return;
        }
        
        contentDiv.innerHTML = '<div class="loading-spinner"><span class="spinner"></span> Lade Abfahrten...</div>';
        
        // Hole die aktuellen Abfahrten vom Panel (TransportModule)
        const depsRes = await fetch('/api/departures');
        const depsData = await depsRes.json();
        
        if (depsData.error) {
            contentDiv.innerHTML = `<div class="error-message">❌ ${depsData.error}</div>`;
            return;
        }
        
        const departures = depsData.departures || [];
        
        if (departures.length === 0) {
            contentDiv.innerHTML = '<div class="no-data">Keine Abfahrten gefunden</div>';
            return;
        }
        
        // Filtere die konfigurierten Linien
        const line1 = config.line1;
        const line2 = config.line2;
        
        let html = `<div class="station-header"><h3>${config.station.name}</h3></div>`;
        
        // Zeige konfigurierte Linien prominent
        if (line1 && line1.name) {
            const departures1 = departures.filter(d => d.line === line1.name && d.direction === line1.dir).slice(0, 2);
            html += renderLineBlock('Linie 1', line1, departures1);
        }
        
        if (line2 && line2.name) {
            const departures2 = departures.filter(d => d.line === line2.name && d.direction === line2.dir).slice(0, 2);
            html += renderLineBlock('Linie 2', line2, departures2);
        }
        
        contentDiv.innerHTML = html;
        
        // Update Zeit anzeigen
        const now = new Date();
        const timeStr = now.toLocaleTimeString('de-CH');
        const updateInfo = document.createElement('div');
        updateInfo.className = 'update-time';
        updateInfo.textContent = `Aktualisiert: ${timeStr}`;
        contentDiv.appendChild(updateInfo);
        
    } catch (e) {
        console.error('Error loading live departures:', e);
        contentDiv.innerHTML = '<div class="error-message">❌ Fehler beim Laden der Abfahrten</div>';
    }
}

function renderLineBlock(title, lineConfig, departures) {
    const icon = getVehicleIcon(getLineType(lineConfig.name, lineConfig.dir));
    let html = `
        <div class="configured-line">
            <div class="line-header">
                <span class="line-title">${title}</span>
                <span class="line-badge" style="background: ${getLineColor(getLineType(lineConfig.name, lineConfig.dir))}">${icon} ${lineConfig.name}</span>
                <span class="line-direction">→ ${lineConfig.dir}</span>
            </div>
    `;
    
    if (departures.length > 0) {
        html += '<div class="departure-list">';
        departures.forEach(dep => {
            const minutes = dep.minutes || 0;
            const timeText = minutes === 0 ? 'Jetzt' : `${minutes}'`;
            html += `<div class="departure-item"><strong>${timeText}</strong></div>`;
        });
        html += '</div>';
    } else {
        html += '<div class="no-departures">Keine aktuellen Abfahrten</div>';
    }
    
    html += '</div>';
    return html;
}

function getLineColor(type) {
    const colors = {
        'tram': '#D32F2F',
        'bus': '#1976D2',
        'train': '#388E3C',
        'metro': '#F57C00',
        'rail': '#7B1FA2',
        'cableway': '#795548'
    };
    return colors[type] || '#757575';
}

function startLiveDeparturesRefresh() {
    // Initial load
    loadLiveDepartures();
    
    // Refresh alle 30 Sekunden
    if (refreshInterval) {
        clearInterval(refreshInterval);
    }
    refreshInterval = setInterval(loadLiveDepartures, 30000);
}

function refreshDepartures() {
    loadLiveDepartures();
}

// =====================
// UX Helper Functions
// =====================
function showToast(message, type = 'info') {
    // Entferne existierenden Toast
    const existingToast = document.querySelector('.toast');
    if (existingToast) {
        existingToast.remove();
    }
    
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    document.body.appendChild(toast);
    
    // Fade in
    setTimeout(() => toast.classList.add('show'), 10);
    
    // Fade out und entfernen
    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}

function showConfirmDialog(message, onConfirm) {
    const confirmed = confirm(message);
    if (confirmed && onConfirm) {
        onConfirm();
    }
    return confirmed;
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
            document.getElementById('wifi-section').style.display = 'block';
            
            // Load current config into form fields
            await loadCurrentConfig(data);
            
            // Wenn eine Haltestelle konfiguriert ist, zeige Live-Abfahrten
            if (data.station && data.station.id && data.line1 && data.line1.name) {
                document.getElementById('live-departures-section').style.display = 'block';
                startLiveDeparturesRefresh();
            }
        } else {
            // Setup Mode
            document.getElementById('app-config-section').style.display = 'none';
            document.getElementById('live-departures-section').style.display = 'none';
        }

    } catch (e) {
        console.error('Status Error', e);
    }
}

async function loadCurrentConfig(data) {
    // Station
    if (data.station && data.station.id) {
        document.getElementById('st_id').value = data.station.id;
        document.getElementById('st_name').value = data.station.name;
        currentStopId = data.station.id;
        
        const selectedDiv = document.getElementById('selected-stop');
        selectedDiv.innerHTML = `<strong>${data.station.name}</strong> <span class="stop-id">(${data.station.id})</span>`;
        selectedDiv.classList.add('has-selection');
        
        // Linien laden
        await loadLinesForStop(data.station.id);
        
        // Danach die ausgewählten Linien setzen
        if (data.line1 && data.line1.name) {
            document.getElementById('l1_name').value = data.line1.name;
            document.getElementById('l1_dir').value = data.line1.dir || '';
            
            // Versuche im Dropdown zu finden und zu setzen
            selectLineInDropdown(1, data.line1.name, data.line1.dir);
        }
        
        if (data.line2 && data.line2.name) {
            document.getElementById('l2_name').value = data.line2.name;
            document.getElementById('l2_dir').value = data.line2.dir || '';
            
            // Versuche im Dropdown zu finden und zu setzen
            selectLineInDropdown(2, data.line2.name, data.line2.dir);
        }
    }
}

function selectLineInDropdown(lineNumber, lineName, direction) {
    const select = document.getElementById(`l${lineNumber}_select`);
    
    // Durchsuche alle Optionen
    for (let i = 0; i < select.options.length; i++) {
        const option = select.options[i];
        if (option.value) {
            try {
                const lineData = JSON.parse(option.value);
                if (lineData.line === lineName && lineData.dir === direction) {
                    select.selectedIndex = i;
                    return;
                }
            } catch (e) {
                // Ignoriere Parse-Fehler
            }
        }
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
    
    // Bestätigung mit verbessertem Dialog
    if (!showConfirmDialog('Konfiguration speichern und Gerät neu starten?')) {
        return;
    }
    
    const btn = document.getElementById('save-btn');
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span> Speichere...';
    
    try {
        const res = await fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        
        const result = await res.json();
        showToast(result.message, 'success');
        
        // Zeige Countdown
        let countdown = 5;
        const interval = setInterval(() => {
            btn.innerHTML = `Neustart in ${countdown}s...`;
            countdown--;
            if (countdown < 0) {
                clearInterval(interval);
                btn.innerHTML = 'Neustart läuft...';
            }
        }, 1000);
        
    } catch (e) {
        showToast('Fehler beim Speichern!', 'error');
        btn.disabled = false;
        btn.textContent = 'Speichern & Neustart';
    }
}

async function factoryReset() {
    if (!showConfirmDialog("Wirklich alle Einstellungen löschen und auf Werkseinstellungen zurücksetzen?\n\nDiese Aktion kann nicht rückgängig gemacht werden!")) {
        return;
    }
    
    try {
        const res = await fetch('/api/reset', { method: 'POST' });
        const result = await res.json();
        showToast(result.message, 'success');
        
        // Lokale Favoriten ebenfalls löschen
        localStorage.removeItem('favoriteStops');
        
        setTimeout(() => {
            showToast('Gerät wird neu gestartet...', 'info');
        }, 1000);
    } catch (e) {
        showToast('Fehler beim Reset!', 'error');
    }
}
