import requests
import re
import os
import sys
from datetime import datetime
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

def test_ojp_request(stop_name, api_key):
    url = "https://api.opentransportdata.swiss/ojp2020"
    
    now = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    
    # XML Payload
    xml_payload = f"""<?xml version="1.0" encoding="UTF-8"?>
<OJP xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="http://www.siri.org.uk/siri" version="1.0" xmlns:ojp="http://www.vdv.de/ojp" xsi:schemaLocation="http://www.siri.org.uk/siri ../ojp-xsd-v1.0/OJP.xsd">
    <OJPRequest>
        <ServiceRequest>
            <RequestTimestamp>{now}</RequestTimestamp>
            <RequestorRef>CrowPanelUser</RequestorRef>
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

    # Auth prefixes to try
    auth_prefixes = ["Bearer ", "", "Token "]
    
    print(f"Testing OJP API at {url}")
    print(f"Stop: {stop_name}")
    print(f"API Key (first 10 chars): {api_key[:10]}...")
    print("-" * 50)

    for prefix in auth_prefixes:
        auth_header = f"{prefix}{api_key}"
        print(f"Trying Authorization: '{prefix}...'")
        
        headers = {
            "Authorization": auth_header,
            "Content-Type": "application/xml", # Try application/xml standard
            "User-Agent": "CrowPanel/1.0"
        }
        
        try:
            response = requests.post(url, headers=headers, data=xml_payload.encode('utf-8'))
            
            if response.status_code == 200:
                print(f"✅ Success with prefix '{prefix}'!")
                print("-" * 50)
                
                # Parse response
                content = response.text
                # Remove namespaces for easier parsing
                content = re.sub(r' xmlns="[^"]+"', '', content, count=1)
                content = re.sub(r' xmlns:([a-zA-Z0-9]+)="[^"]+"', '', content)
                content = re.sub(r'([a-zA-Z0-9]+):', '', content)
                
                try:
                    root = ET.fromstring(content)
                    locations = root.findall(".//Location")
                    if not locations:
                        print("No locations found in response.")
                    
                    for loc in locations:
                        name_node = loc.find(".//StopPlaceName/Text")
                        ref_node = loc.find(".//StopPlaceRef")
                        
                        name = name_node.text if name_node is not None else "Unknown"
                        ref = ref_node.text if ref_node is not None else "Unknown"
                        print(f"Found: {name} (ID: {ref})")
                    
                    # Exit after success
                    return
                    
                except Exception as e:
                    print(f"Error parsing XML: {e}")
            else:
                print(f"❌ Failed: {response.status_code}")
                if response.status_code == 403:
                    print(f"   Response: {response.text.strip()}")
                
        except Exception as e:
            print(f"   Exception: {e}")
        
        print("-" * 20)

    print("\n⚠️  If all attempts failed with 403:")
    print("1. Check if the API Key in secrets.h matches the 'Token' on the portal exactly.")
    print("2. It can take up to 60 minutes for a newly created key to become active.")
    print("3. Verify you are using the 'OJP 2.0' API endpoint (already set in script).")

if __name__ == "__main__":
    key = get_api_key()
    
    if len(sys.argv) > 1:
        search_term = " ".join(sys.argv[1:])
    else:
        search_term = "Zürich, Hauptbahnhof"
    
    test_ojp_request(search_term, key)
