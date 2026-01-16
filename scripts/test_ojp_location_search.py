#!/usr/bin/env python3
"""
OpenTransportData Swiss OJP API Test Script
Tests both OJP 1.0 and OJP 2.0 endpoints with correct XML formats.
"""

import requests
import re
import os
import sys
import base64
from datetime import datetime, timezone
import xml.etree.ElementTree as ET

# Path to secrets.h
SECRETS_PATH = os.path.join(os.path.dirname(__file__), '../include/secrets.h')

def get_api_key():
    """Extracts the API key from secrets.h"""
    try:
        with open(SECRETS_PATH, 'r') as f:
            content = f.read()
            match = re.search(r'#define\s+OJP_API_KEY\s+"([^"]+)"', content)
            if match:
                return match.group(1)
            else:
                print("Error: OJP_API_KEY not found in secrets.h")
                sys.exit(1)
    except FileNotFoundError:
        print(f"Error: {SECRETS_PATH} not found.")
        sys.exit(1)

def check_key_format(api_key):
    """Checks if the key looks like the Token Hash instead of the Token"""
    print(f"API Key length: {len(api_key)} characters")
    print(f"API Key preview: {api_key[:20]}...{api_key[-10:]}")
    
    if api_key.startswith("eyJ"):
        try:
            padded = api_key + '=' * (-len(api_key) % 4)
            decoded = base64.b64decode(padded).decode('utf-8')
            if '"h":"murmur128"' in decoded or '"h":' in decoded:
                print(f"\n⚠️  WARNUNG: Key könnte der 'Token Hash' sein!")
                print(f"    Dekodiert: {decoded}")
                print("    ABER: Laut Support zeigt das Portal manchmal denselben Wert.")
                print("    Wir testen trotzdem weiter...")
                print("-" * 60)
        except:
            pass
    return True

def build_ojp2_location_request(stop_name):
    """Builds OJP 2.0 LocationInformationRequest XML"""
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<OJP xmlns="http://www.vdv.de/ojp" xmlns:siri="http://www.siri.org.uk/siri" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="2.0">
    <OJPRequest>
        <siri:ServiceRequest>
            <siri:ServiceRequestContext>
                <siri:Language>de</siri:Language>
            </siri:ServiceRequestContext>
            <siri:RequestTimestamp>{now}</siri:RequestTimestamp>
            <siri:RequestorRef>CrowPanel</siri:RequestorRef>
            <OJPLocationInformationRequest>
                <siri:RequestTimestamp>{now}</siri:RequestTimestamp>
                <siri:MessageIdentifier>LocationSearch1</siri:MessageIdentifier>
                <InitialInput>
                    <Name>{stop_name}</Name>
                </InitialInput>
                <Restrictions>
                    <Type>stop</Type>
                    <NumberOfResults>10</NumberOfResults>
                    <IncludePtModes>true</IncludePtModes>
                </Restrictions>
            </OJPLocationInformationRequest>
        </siri:ServiceRequest>
    </OJPRequest>
</OJP>"""

def build_ojp1_location_request(stop_name):
    """Builds OJP 1.0 LocationInformationRequest XML (for ojp2020 endpoint)"""
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<OJP xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
     xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
     xmlns="http://www.siri.org.uk/siri" 
     version="1.0" 
     xmlns:ojp="http://www.vdv.de/ojp">
    <OJPRequest>
        <ServiceRequest>
            <RequestTimestamp>{now}</RequestTimestamp>
            <RequestorRef>CrowPanel</RequestorRef>
            <ojp:OJPLocationInformationRequest>
                <RequestTimestamp>{now}</RequestTimestamp>
                <ojp:InitialInput>
                    <ojp:LocationName>{stop_name}</ojp:LocationName>
                </ojp:InitialInput>
                <ojp:Restrictions>
                    <ojp:Type>stop</ojp:Type>
                    <ojp:NumberOfResults>5</ojp:NumberOfResults>
                </ojp:Restrictions>
            </ojp:OJPLocationInformationRequest>
        </ServiceRequest>
    </OJPRequest>
</OJP>"""

def parse_location_response(response_text):
    """Parse the XML response and extract stop information"""
    # Remove namespaces for easier parsing
    content = response_text
    content = re.sub(r' xmlns(:[a-zA-Z0-9]+)?="[^"]+"', '', content)
    content = re.sub(r'<([a-zA-Z0-9]+):', '<', content)
    content = re.sub(r'</([a-zA-Z0-9]+):', '</', content)
    
    try:
        root = ET.fromstring(content)
        
        # In OJP 2.0, results are in PlaceResult elements
        place_results = root.findall(".//PlaceResult")
            
        results = []
        for place in place_results:
            # Get the Place element
            place_elem = place.find(".//Place")
            if place_elem is None:
                continue
                
            # Try to find stop place info
            stop_place = place_elem.find(".//StopPlace")
            if stop_place is not None:
                # Get StopPlaceRef (the ID we need!)
                ref_node = stop_place.find("StopPlaceRef")
                # Get StopPlaceName
                name_node = stop_place.find(".//StopPlaceName/Text")
                # Get TopographicPlaceName for context
                topo_node = stop_place.find(".//TopographicPlaceName/Text")
                
                ref = ref_node.text if ref_node is not None else "Unknown"
                name = name_node.text if name_node is not None else "Unknown"
                topo = topo_node.text if topo_node is not None else ""
                
                display_name = f"{name} ({topo})" if topo else name
                results.append((display_name, ref))
        
        return results
    except Exception as e:
        print(f"XML Parse Error: {e}")
        import traceback
        traceback.print_exc()
        return []

def test_endpoint(url, xml_payload, api_key, endpoint_name):
    """Test a specific endpoint with the given XML payload"""
    print(f"\n{'='*60}")
    print(f"Testing: {endpoint_name}")
    print(f"URL: {url}")
    print(f"{'='*60}")
    
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/xml",
        "User-Agent": "CrowPanel-OEV-Display/1.0",
        "Accept": "application/xml"
    }
    
    try:
        response = requests.post(url, headers=headers, data=xml_payload.encode('utf-8'), timeout=30)
        
        print(f"Status Code: {response.status_code}")
        print(f"Response Headers: {dict(response.headers)}")
        
        if response.status_code == 200:
            print("\n✅ SUCCESS!")
            results = parse_location_response(response.text)
            if results:
                print("\nGefundene Haltestellen:")
                for name, ref in results:
                    print(f"  • {name} (ID: {ref})")
            else:
                print("\nKeine Haltestellen in der Antwort gefunden.")
                print("Raw Response (first 1000 chars):")
                print(response.text[:1000])
            return True
        elif response.status_code == 400:
            print("\n❌ 400 Bad Request - XML Format möglicherweise falsch")
            print(f"Response: {response.text[:500]}")
        elif response.status_code == 401:
            print("\n❌ 401 Unauthorized - API Key ungültig oder fehlt")
            print(f"Response: {response.text}")
        elif response.status_code == 403:
            print("\n❌ 403 Forbidden - Zugriff verweigert")
            print(f"Response: {response.text}")
        elif response.status_code == 404:
            print("\n❌ 404 Not Found - Endpoint existiert nicht")
        else:
            print(f"\n❌ HTTP Error {response.status_code}")
            print(f"Response: {response.text[:500]}")
            
    except requests.exceptions.Timeout:
        print("\n❌ Timeout - Server antwortet nicht")
    except requests.exceptions.ConnectionError as e:
        print(f"\n❌ Connection Error: {e}")
    except Exception as e:
        print(f"\n❌ Exception: {e}")
    
    return False

def main():
    print("=" * 60)
    print("OpenTransportData Swiss OJP API Test")
    print("=" * 60)
    
    api_key = get_api_key()
    check_key_format(api_key)
    
    if len(sys.argv) > 1:
        stop_name = " ".join(sys.argv[1:])
    else:
        stop_name = "Bern"
    
    print(f"\nSuche nach: '{stop_name}'")
    
    # Test configurations
    tests = [
        # OJP 2.0 endpoint with OJP 2.0 XML
        ("https://api.opentransportdata.swiss/ojp20", 
         build_ojp2_location_request(stop_name), 
         "OJP 2.0 Endpoint + OJP 2.0 XML"),
        
        # OJP 1.0 endpoint with OJP 1.0 XML (fallback)
        ("https://api.opentransportdata.swiss/ojp2020", 
         build_ojp1_location_request(stop_name), 
         "OJP 1.0 Endpoint + OJP 1.0 XML"),
    ]
    
    success = False
    for url, xml, name in tests:
        if test_endpoint(url, xml, api_key, name):
            success = True
            break
    
    if not success:
        print("\n" + "=" * 60)
        print("❌ ALLE TESTS FEHLGESCHLAGEN")
        print("=" * 60)
        print("""
Mögliche Ursachen:
1. Der API-Key ist nicht korrekt oder nicht aktiviert
2. Der Key ist für ein anderes API-Produkt registriert
3. Es gibt ein Problem auf Seiten von OpenTransportData

Nächste Schritte:
1. Prüfe im API-Manager, ob dein Key WIRKLICH für 'OJP 2.0' freigeschaltet ist
2. Versuche den Key zu regenerieren (revoke + neu erstellen)
3. Kontaktiere den Support mit den Infos oben
        """)

if __name__ == "__main__":
    main()
