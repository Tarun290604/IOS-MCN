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
"GNBCUCPFunction": {
"gNBId": @current_gnbid@,
"gNBIdLength": 4,
"mcc": "001",
"mnc": "01"
  },
"NRCellCUGrp": {
"CellLocalID": 3589,
"mcc": "001",
"mnc": "01",
"sst": 1,
"sd": 16777215
},
"Additional": {
"NUM_DUS": 0,
"NUM_CUUPS": 0,
"vendor": "OpenairInterface"
   }
  }
OK"""

running_number = 0
modem_state = 1
new_gnbid = 234
current_gnbid = 1234

def execute_command(writer, command):
    global modem_state
    global o1_stats_template
    global running_number
    global new_gnbid
    global current_gnbid

    print('\'' + command + '\'')

    response = 'ERROR: command not found'
    if command == 'o1 stats':
        response = o1_stats_template.replace("@current_gnbid@", str(current_gnbid))
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
        current_gnbid = new_gnbid
        modem_state = 1
        time.sleep(2)
      else:
        response = 'FAILURE'
        
    elif command.startswith('o1 gnbid '):
      if modem_state == 0:
        number_string = command[len('o1 gnbid'):]
        try:
            new_gnbid = int(number_string)
            response = 'OK'
            time.sleep(1)
        except ValueError:
            response = "FAILURE: parsing new gnbid"
      else:
        response = "FAILURE: modem still running"
  
    response = response.replace("\n", "\r\n")

    print('o1 stats')
    print('\'****' + response + '\'')

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
