#! /usr/bin/env python

#
# Copyright (C) 2008  Lorenzo Pallara, l.pallara@avalpa.com
# (c) 2016 The DVB-TX-IRIS team, University of Perugia
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#                                  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#                                  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os

from dvbobjects.PSI.PAT import *
from dvbobjects.PSI.NIT import *
from dvbobjects.PSI.SDT import *
from dvbobjects.PSI.PMT import *
from dvbobjects.DVB.Descriptors import *
from dvbobjects.MPEG.Descriptors import *
from dvbobjects.PSI.TDT import *
from dvbobjects.PSI.EIT import *


#
# Shared values
#
wishful_transport_stream_id = 1 # demo value
wishful_original_transport_stream_id = 1 # demo value

wishful1hd_service_id = 1 # demo value
wishful1hd_pmt_pid = 1031

wishful2hd_service_id = 2 # demo value
wishful2hd_pmt_pid = 1032

wishful3hd_service_id = 3 # demo value
wishful3hd_pmt_pid = 1033


#
# Network Information Table
# this is a basic NIT with the minimum descriptors, OpenCaster has a big library ready to use
#
nit = network_information_section(
	network_id = 1,
	network_descriptor_loop = [
		network_descriptor(network_name = "WiSHFUL",), 
	],
	transport_stream_loop = [
		transport_stream_loop_item(
			transport_stream_id = wishful_transport_stream_id,
			original_network_id = wishful_original_transport_stream_id,
			transport_descriptor_loop = [
				service_list_descriptor(
					dvb_service_descriptor_loop = [
						service_descriptor_loop_item(
							service_ID = wishful1hd_service_id, 
							service_type = 0x19, # advanced tv service type
						),
						service_descriptor_loop_item(
							service_ID = wishful2hd_service_id, 
							service_type = 0x19, # advanced tv service type
						),
						service_descriptor_loop_item(
							service_ID = wishful3hd_service_id, 
							service_type = 0x19, # advanced tv service type
						),
					],
				),
			],		
		),
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)


#
# Program Association Table (ISO/IEC 13818-1 2.4.4.3)
#
pat = program_association_section(
	transport_stream_id = wishful_transport_stream_id,
	program_loop = [
		program_loop_item(
			program_number = wishful1hd_service_id,
			PID = wishful1hd_pmt_pid,
		),  
		program_loop_item(
			program_number = wishful2hd_service_id,
			PID = wishful2hd_pmt_pid,
		),  
		program_loop_item(
			program_number = wishful3hd_service_id,
			PID = wishful3hd_pmt_pid,
		),  
		program_loop_item(
			program_number = 0, # special program for the NIT
			PID = 16,
		), 
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)


#
# Service Description Table (ETSI EN 300 468 5.2.3) 
# this is a basic SDT with the minimum descriptors, OpenCaster has a big library ready to use
#
sdt = service_description_section(
		transport_stream_id = wishful_transport_stream_id,
		original_network_id = wishful_original_transport_stream_id,	
		service_loop = [
			service_loop_item(
				service_ID = wishful1hd_service_id,
				EIT_schedule_flag = 1, # 0 no current event information is broadcast, 1 broadcast
				EIT_present_following_flag = 0, # 0 no next event information is broadcast, 1 is broadcast
				running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
				free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
				service_descriptor_loop = [
					service_descriptor(
						service_type = 0x19, # advanced television service
						service_provider_name = "WiSHFUL",
						service_name = "WiSHFUL 1 HD",
					),    
				],
			),		
			service_loop_item(
				service_ID = wishful2hd_service_id,
				EIT_schedule_flag = 1, # 0 no current event information is broadcast, 1 broadcast
				EIT_present_following_flag = 0, # 0 no next event information is broadcast, 1 is broadcast
				running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
				free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
				service_descriptor_loop = [
					service_descriptor(
						service_type = 0x19, # advanced television service
						service_provider_name = "WiSHFUL",
						service_name = "WiSHFUL 2 HD",
					),    
				],
			),		
			service_loop_item(
				service_ID = wishful3hd_service_id,
				EIT_schedule_flag = 1, # 0 no current event information is broadcast, 1 broadcast
				EIT_present_following_flag = 0, # 0 no next event information is broadcast, 1 is broadcast
				running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
				free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
				service_descriptor_loop = [
					service_descriptor(
						service_type = 0x19, # advanced television service
						service_provider_name = "WiSHFUL",
						service_name = "WiSHFUL 3 HD",
					),    
				],
			),		
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)

#
# Program Map Table (ISO/IEC 13818-1 2.4.4.8)
# this is a basic PMT the the minimum descriptors, OpenCaster has a big library ready to use
#	
pmt1hd = program_map_section(
	program_number = wishful1hd_service_id,
	PCR_PID = 2064,
	program_info_descriptor_loop = [],
	stream_loop = [
		stream_loop_item(
			stream_type = 0x1B, # avc video stream type
			elementary_PID = 2064,
			element_info_descriptor_loop = []
		),
		stream_loop_item(
			stream_type = 4, # mpeg2 audio stream type
			elementary_PID = 2068,
			element_info_descriptor_loop = []
		),
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)          
   
pmt2hd = program_map_section(
	program_number = wishful2hd_service_id,
	PCR_PID = 2074,
	program_info_descriptor_loop = [],
	stream_loop = [
		stream_loop_item(
			stream_type = 0x1B, # avc video stream type
			elementary_PID = 2074,
			element_info_descriptor_loop = []
		),
		stream_loop_item(
			stream_type = 4, # mpeg2 audio stream type
			elementary_PID = 2078,
			element_info_descriptor_loop = []
		),
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)          
   
pmt3hd = program_map_section(
	program_number = wishful3hd_service_id,
	PCR_PID = 2084,
	program_info_descriptor_loop = [],
	stream_loop = [
		stream_loop_item(
			stream_type = 0x1B, # avc video stream type
			elementary_PID = 2084,
			element_info_descriptor_loop = []
		),
		stream_loop_item(
			stream_type = 4, # mpeg2 audio stream type
			elementary_PID = 2088,
			element_info_descriptor_loop = []
		),
	],
	version_number = 1, # you need to change the table number every time you edit, so the decoder
						# will compare its version with the new one and update the table
	section_number = 0,
	last_section_number = 0,
)          

#
# Time Description Table (ETSI EN 300 468 5.2.5) 
# it should be replaced at run time with tstdt
#
tdt = time_date_section(
	year = 116, # since 1900
	month = 7,
	day = 6,
	hour = 0x11, # use hex like decimals
	minute = 0x30,
	second = 0x00,
	version_number = 1,
	section_number = 0,
	last_section_number = 0,
)
	
#
# Event Information Table (ETSI EN 300 468 5.2.4) 
#
eit1hd = event_information_section(
	table_id = EIT_ACTUAL_TS_PRESENT_FOLLOWING,
	service_id = wishful1hd_service_id,
	transport_stream_id = wishful_transport_stream_id,
	original_network_id = wishful_original_transport_stream_id,
    event_loop = [
	    event_loop_item(
			event_id = 1,
			start_year = 116, # since 1900
			start_month = 7, 
			start_day = 6, 
			start_hours = 0x12, # use hex like decimals
			start_minutes = 0x30,
			start_seconds = 0x00,
			duration_hours = 0x06,
			duration_minutes = 0x00,
			duration_seconds = 0x00,
			running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
			free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
			event_descriptor_loop = [
				short_event_descriptor (
					ISO639_language_code = "ENG", 
					event_name = "Tears Of Steel (project Mango)",
					text = "The short science fiction film is about a group of warriors and scientists who gather in a future Amsterdam to stage a crucial event from the past in a desperate attempt to rescue the world from destructive robots.",
				),
			],
	    ),	    	
    ],
	segment_last_section_number = 1,
	version_number = 1,
	section_number = 0,
	last_section_number = 0, # pay attention here, we have another section after this!
)

eit2hd = event_information_section(
	table_id = EIT_ACTUAL_TS_PRESENT_FOLLOWING,
	service_id = wishful2hd_service_id,
	transport_stream_id = wishful_transport_stream_id,
	original_network_id = wishful_original_transport_stream_id,
    event_loop = [
	    event_loop_item(
			event_id = 2,
			start_year = 116, # since 1900
			start_month = 7, 
			start_day = 6, 
			start_hours = 0x12, # use hex like decimals
			start_minutes = 0x30,
			start_seconds = 0x00,
			duration_hours = 0x06,
			duration_minutes = 0x00,
			duration_seconds = 0x00,
			running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
			free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
			event_descriptor_loop = [
				short_event_descriptor (
					ISO639_language_code = "ENG", 
					event_name = "Sintel (project Durian)",
					text = "A woman, Sintel, is attacked while traveling through a wintry mountainside. She finds refuge in a shaman's hut. She confesses that she is looking for a dragon she nursed him back to health and named him Scales.",
				),
			],
	    ),	    	
    ],
	segment_last_section_number = 1,
	version_number = 1,
	section_number = 0,
	last_section_number = 0, # pay attention here, we have another section after this!
)

eit3hd = event_information_section(
	table_id = EIT_ACTUAL_TS_PRESENT_FOLLOWING,
	service_id = wishful3hd_service_id,
	transport_stream_id = wishful_transport_stream_id,
	original_network_id = wishful_original_transport_stream_id,
    event_loop = [
	    event_loop_item(
			event_id = 3,
			start_year = 116, # since 1900
			start_month = 7, 
			start_day = 6, 
			start_hours = 0x12, # use hex like decimals
			start_minutes = 0x30,
			start_seconds = 0x00,
			duration_hours = 0x06,
			duration_minutes = 0x00,
			duration_seconds = 0x00,
			running_status = 4, # 4 service is running, 1 not running, 2 starts in a few seconds, 3 pausing
			free_CA_mode = 0, # 0 means service is not scrambled, 1 means at least a stream is scrambled
			event_descriptor_loop = [
				short_event_descriptor (
					ISO639_language_code = "ENG", 
					event_name = "Big Buck Bunny (project Peach)",
					text = "The movie follows a day of the life of Big Buck Bunny when he meets three bullying rodents: Frank, Rinky and Gimera. The rodents amuse themselves by harassing helpless creatures. BBB avenges them!",
				),
			],
	    ),	    	
    ],
	segment_last_section_number = 1,
	version_number = 1,
	section_number = 0,
	last_section_number = 0, # pay attention here, we have another section after this!
)


#
# PSI marshalling and encapsulation
#
out = open("./nit.sec", "wb")
out.write(nit.pack())
out.close
out = open("./nit.sec", "wb") # python  flush bug
out.close
os.system('sec2ts 16 < ./nit.sec > ./nit.ts')

out = open("./pat.sec", "wb")
out.write(pat.pack())
out.close
out = open("./pat.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 0 < ./pat.sec > ./pat.ts')

out = open("./sdt.sec", "wb")
out.write(sdt.pack())
out.close
out = open("./sdt.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 17 < ./sdt.sec > ./sdt.ts')

out = open("./pmt1.sec", "wb")
out.write(pmt1hd.pack())
out.close
out = open("./pmt1.sec", "wb") # python   flush bug
out.close
os.system('sec2ts ' + str(wishful1hd_pmt_pid) + ' < ./pmt1.sec > ./pmt1.ts')

out = open("./pmt2.sec", "wb")
out.write(pmt2hd.pack())
out.close
out = open("./pmt2.sec", "wb") # python   flush bug
out.close
os.system('sec2ts ' + str(wishful2hd_pmt_pid) + ' < ./pmt2.sec > ./pmt2.ts')

out = open("./pmt3.sec", "wb")
out.write(pmt3hd.pack())
out.close
out = open("./pmt3.sec", "wb") # python   flush bug
out.close
os.system('sec2ts ' + str(wishful3hd_pmt_pid) + ' < ./pmt3.sec > ./pmt3.ts')

out = open("./tdt.sec", "wb")
out.write(tdt.pack())
out.close
out = open("./tdt.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 20 < ./tdt.sec > ./tdt.ts')

out = open("./eit1.sec", "wb")
out.write(eit1hd.pack())
out.close
out = open("./eit1.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 18 < ./eit1.sec > ./eit1.ts')

out = open("./eit2.sec", "wb")
out.write(eit2hd.pack())
out.close
out = open("./eit2.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 18 < ./eit2.sec > ./eit2.ts')

out = open("./eit3.sec", "wb")
out.write(eit3hd.pack())
out.close
out = open("./eit3.sec", "wb") # python   flush bug
out.close
os.system('sec2ts 18 < ./eit3.sec > ./eit3.ts')

