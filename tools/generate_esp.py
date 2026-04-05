#!/usr/bin/env python3
"""
Generate SkyUIRecentSort.esp with MCM quest structure.
This creates a minimal ESP plugin compatible with Skyrim SE/AE.
"""

import struct
import sys
from pathlib import Path

# Form IDs
SKYUI_ESP_FORMID = 0x000800  # SkyUI.esp master file marker
QUEST_FORMID = 0x00000800    # Our quest FormID

def write_string(f, s, fixed_length=None):
    """Write null-terminated string."""
    encoded = s.encode('windows-1252')
    if fixed_length:
        encoded = encoded[:fixed_length].ljust(fixed_length, b'\x00')
    else:
        encoded += b'\x00'
    f.write(encoded)

def write_record(f, record_type, flags, formid, subrecords):
    """Write a TES4/Skyrim record."""
    # Calculate size
    data = b''.join(subrecords)
    size = len(data)
    
    f.write(record_type.encode('ascii'))
    f.write(struct.pack('<I', size))      # dataSize
    f.write(struct.pack('<I', flags))     # flags
    f.write(struct.pack('<I', formid))    # formID
    f.write(struct.pack('<I', 0))         # revision (timestamp1)
    f.write(struct.pack('<H', 0))         # version
    f.write(struct.pack('<H', 0))         # unknown
    f.write(data)

def create_subrecord(subtype, data):
    """Create a subrecord."""
    result = subtype.encode('ascii')
    result += struct.pack('<H', len(data))
    result += data
    return result

def generate_esp(output_path):
    """Generate the ESP file."""
    
    with open(output_path, 'wb') as f:
        # TES4 Header
        tes4_subs = []
        
        # HEDR - Header
        hedr_data = struct.pack('<f', 1.70)  # version 1.70 (Skyrim SE)
        hedr_data += struct.pack('<I', 1)     # numRecords
        hedr_data += struct.pack('<I', QUEST_FORMID + 1)  # nextObjectID
        tes4_subs.append(create_subrecord('HEDR', hedr_data))
        
        # CNAM - Author
        tes4_subs.append(create_subrecord('CNAM', b'SkyUIRecentSort\x00'))
        
        # SNAM - Description
        tes4_subs.append(create_subrecord('SNAM', b'MCM for SkyUI Recent Sort\x00'))
        
        # MAST - Master file (SkyUI.esp)
        tes4_subs.append(create_subrecord('MAST', b'SkyUI_SE.esp\x00'))
        
        # DATA - Master file size (dummy value, game doesn't validate this)
        tes4_subs.append(create_subrecord('DATA', struct.pack('<Q', 0)))
        
        write_record(f, 'TES4', 0, 0, tes4_subs)
        
        # QUST Record - Quest
        qust_subs = []
        
        # EDID - Editor ID
        qust_subs.append(create_subrecord('EDID', b'SkyUIRecentSort_Quest\x00'))
        
        # VMAD - Virtual Machine Adapter (Papyrus scripts)
        vmad = b''
        vmad += struct.pack('<H', 5)  # version
        vmad += struct.pack('<H', 2)  # objFormat
        vmad += struct.pack('<H', 2)  # scriptCount
        
        # Script 1: SkyUIRecentSort_Quest
        script1_name = b'SkyUIRecentSort_Quest\x00'
        vmad += struct.pack('<H', len(script1_name) - 1)
        vmad += script1_name
        vmad += struct.pack('<B', 0)  # flags
        vmad += struct.pack('<H', 1)  # propertyCount
        
        # Property: MCM
        prop_name = b'MCM\x00'
        vmad += struct.pack('<H', len(prop_name) - 1)
        vmad += prop_name
        vmad += struct.pack('<B', 4)  # type = Object
        vmad += struct.pack('<B', 1)  # flags (edited)
        vmad += struct.pack('<H', 0)  # alias (none)
        vmad += struct.pack('<H', 1)  # object union - script index
        
        # Script 2: SkyUIRecentSort_MCM (SKI_ConfigBase)
        script2_name = b'SkyUIRecentSort_MCM\x00'
        vmad += struct.pack('<H', len(script2_name) - 1)
        vmad += script2_name
        vmad += struct.pack('<B', 0)  # flags
        vmad += struct.pack('<H', 0)  # propertyCount
        
        qust_subs.append(create_subrecord('VMAD', vmad))
        
        # FULL - Name
        qust_subs.append(create_subrecord('FULL', b'SkyUI Recent Sort\x00'))
        
        # DNAM - General flags
        dnam = struct.pack('<H', 0x0001)  # Start Game Enabled
        qust_subs.append(create_subrecord('DNAM', dnam))
        
        # ENAM - Event
        qust_subs.append(create_subrecord('ENAM', struct.pack('<I', 0)))
        
        # QTGL - Quest flags (runtime data)
        qust_subs.append(create_subrecord('QTGL', struct.pack('<I', 0)))
        
        # FLTR - Filter (MCM)
        qust_subs.append(create_subrecord('FLTR', b'SkyUIRecentSort\x00'))
        
        write_record(f, 'QUST', 0x00004000, QUEST_FORMID, qust_subs)  # flags: Persistent
    
    print(f"Generated ESP: {output_path}")
    print(f"  Quest FormID: {QUEST_FORMID:08X}")
    print(f"  Master: SkyUI_SE.esp")

if __name__ == '__main__':
    script_dir = Path(__file__).parent
    output = script_dir.parent / 'SkyUIRecentSort.esp'
    
    try:
        generate_esp(output)
        print(f"\nESP file created successfully!")
        print(f"Copy to your Skyrim Data folder: {output}")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
