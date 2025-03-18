#!/bin/python3

# /*
# * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# * contributor license agreements.  See the NOTICE file distributed with
# * this work for additional information regarding copyright ownership.
# * The OpenAirInterface Software Alliance licenses this file to You under
# * the OAI Public License, Version 1.1  (the "License"); you may not use this file
# * except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.openairinterface.org/?page_id=698
# *
# * Copyright: Fraunhofer Heinrich Hertz Institute
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *-------------------------------------------------------------------------------
# * For more information about the OpenAirInterface (OAI) Software Alliance:
# *      contact@openairinterface.org
# */


import asyncio, telnetlib3, time

o1_stats_template = """{
  "o1-config": {
    "BWP": {
      "dl": [
        {
          "bwp3gpp:isInitialBwp": true,
          "bwp3gpp:numberOfRBs": 106,
          "bwp3gpp:startRB": 0,
          "bwp3gpp:subCarrierSpacing": 30
        }
      ],
      "ul": [
        {
          "bwp3gpp:isInitialBwp": true,
          "bwp3gpp:numberOfRBs": 106,
          "bwp3gpp:startRB": 0,
          "bwp3gpp:subCarrierSpacing": 30
        }
      ]
    },
    "NRCELLDU": {
      "nrcelldu3gpp:ssbFrequency": 641280,
      "nrcelldu3gpp:arfcnDL": 640008,
      "nrcelldu3gpp:bSChannelBwDL": @current_bandwidth@,
      "nrcelldu3gpp:arfcnUL": 640008,
      "nrcelldu3gpp:bSChannelBwUL": @current_bandwidth@,
      "nrcelldu3gpp:nRPCI": 0,
      "nrcelldu3gpp:nRTAC": 1,
      "nrcelldu3gpp:mcc": "208",
      "nrcelldu3gpp:mnc": "95",
      "nrcelldu3gpp:sd": 16777215,
      "nrcelldu3gpp:sst": 1
    },
    "o-ru-stats": {
    "o-ran-uplane-conf": [
    {
      "tx-array-carrier:absolute-frequency-center": 641280,
      "tx-array-carrier:center-of-channel-bandwidth": 3700020000,
      "tx-array-carrier:channel-bandwidth": 100000000,
      "tx-array-carrier:active": "INACTIVE",
      "tx-array-carrier:rw-duplex-scheme": "TDD",
      "tx-array-carrier:rw-type": "NR",
      "tx-array-carrier:gain": 27,
      "tx-array-carrier:downlink-radio-frame-offset": 0,
      "tx-array-carrier:downlink-sfn-offset": 0
    },
    {
      "rx-array-carrier:absolute-frequency-center": 641280,
      "rx-array-carrier:center-of-channel-bandwidth": 3700020000,
      "rx-array-carrier:channel-bandwidth": 100000000,
      "rx-array-carrier:active": "INACTIVE",
      "rx-array-carrier:downlink-radio-frame-offset": 0,
      "rx-array-carrier:downlink-sfn-offset": 0,
      "rx-array-carrier:gain-correction": 0.0,
      "rx-array-carrier:n-ta-offset": 25600
      }
    ],
    "delay-management": {
      "ru-delay-profile:t2a-min-up": 105000, 
      "ru-delay-profile:t2a-max-up": 375000, 
      "ru-delay-profile:t2a-min-cp-dl": 200000, 
      "ru-delay-profile:t2a-max-cp-dl": 233000, 
      "ru-delay-profile:tcp-adv-dl": 125000, 
      "ru-delay-profile:ta3-min": 75000,
      "ru-delay-profile:ta3-max": 108000,
      "ru-delay-profile:t2a-min-cp-ul": 125000,
      "ru-delay-profile:t2a-max-cp-ul": 336000
    }
    },
    "device": {
      "gnbId": 3584,
      "gnbName": "gNB-Eurecom-5GNRBox",
      "vendor": "OpenAirInterface Software Alliance"
    }
  },
  "O1-Operational": {
    "frame-type": "tdd",
    "band-number": 78,
    "num-ues": 2,
    "ues": [
      6876,
      28734
    ],
    "load": 12,
    "ues-thp": [
      {
        "rnti": 6876,
        "dl": 3279,
        "ul": 2725
      },
      {
        "rnti": 28734,
        "dl": 1279,
        "ul": 5725
      }
    ]
  }
}
OK"""

running_number = 0
current_bandwidth = 40
new_bandwidth = 40
modem_state = 1

def execute_command(writer, command):
    global current_bandwidth
    global new_bandwidth
    global modem_state
    global o1_stats_template
    global running_number

    print('\'' + command + '\'')

    response = 'ERROR: command not found'
    if command == 'o1 stats':
        response = o1_stats_template.replace("@current_bandwidth@", str(current_bandwidth))
        response = response.replace("@running_number@", str(running_number))
        running_number = running_number + 1
        

    elif command == 'o1 stop_modem':
      if modem_state == 1:
        modem_state = 0
        response = 'OK'
        time.sleep(1)
      else:
        response = 'FAILURE'

    elif command == 'o1 start_modem':
      if modem_state == 0:
        response = 'OK'
        current_bandwidth = new_bandwidth
        modem_state = 1
        time.sleep(2)
      else:
        response = 'FAILURE'
        
    elif command.startswith('o1 bwconfig '):
      if modem_state == 0:
        number_string = command[len('o1 bwconfig '):]
        try:
            new_bandwidth = int(number_string)
            response = 'OK'
            time.sleep(1)
        except ValueError:
            response = "FAILURE: parsing new bw"
      else:
        response = "FAILURE: modem still running"
  
    response = response.replace("\n", "\r\n")
    writer.write("\r\n" + response + "\r\n")

async def shell(reader, writer):
    print('connection opened...')
    writer.write('softmodem_gnb> ')
    await writer.drain()
    command = ''

    while True:
        inp = await reader.read(1)
        if inp:
            command += inp
            writer.echo(inp)
            if (command.endswith("\r") or command.endswith("\n")):
                command = command.strip("\r\n ")

                print("Received command: " + command)
                # writer.write("softmodem_gnb> " + command + "\r\n")
                await writer.drain()
                execute_command(writer, command)

                writer.write('softmodem_gnb> ')
                await writer.drain()
                
                command = ""
        else:
          break
        
    writer.close()
    reader.close()
    print('connection closed...')
    

loop = asyncio.get_event_loop()
coro = telnetlib3.create_server(port=9090, shell=shell)
server = loop.run_until_complete(coro)
loop.run_until_complete(server.wait_closed())
