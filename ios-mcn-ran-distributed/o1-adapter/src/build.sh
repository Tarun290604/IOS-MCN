#!/bin/bash

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

files=(
    # alarms
    "alarms/alarms.c"

    # common
    "common/config.c"
    "common/utils.c"

    # netconf
    "netconf/netconf.c"
    "netconf/netconf_session.c"
    "netconf/netconf_data.c"
    
    # oai
    "oai/oai.c"
    "oai/oai_data.c"

    # pm_data
    "pm_data/pm_data.c"

    # telnet
    "telnet/telnet.c"

    # ves
    "ves/ves.c"
    "ves/ves_internal.c"

    "main.c"
)

include=(
    "."
)

libs=(
    "pthread"
    "yang"
    "sysrepo"
    "curl"
    "telnet"
    "cjson"
)

sources=""
for i in ${files[@]}
do
    sources="$sources $i"
done

libraries=""
for i in ${libs[@]}
do
    libraries="$libraries -l$i"
done

includes=""
for i in ${other_include[@]}
do
    includes="$includes -I$i"
done

for i in ${include[@]}
do
    includes="$includes -I$i"
done

output="gnb-adapter"
build="gcc -g -Wall -pedantic $includes $sources $libraries -o$output"

clear
echo "Building with command: $build"
$build
if [ "$?" -ne "0" ]; then
  echo "Build failed"
  exit 1
fi

exit $?