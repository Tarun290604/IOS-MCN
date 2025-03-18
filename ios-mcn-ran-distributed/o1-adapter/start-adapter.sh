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

START_DEV="false"
START_ADAPTER="false"
GO="false"

until [ -z "$1" ]
do
    case "$1" in 
        --dev)
            START_DEV="true"
            shift
            ;;

        --adapter)
            START_ADAPTER="true"
            shift
            ;;

        --go)
            GO="true"
            shift
            ;;

        *)
            shift
            ;;
    esac
done

if $START_DEV; then
    if $GO; then
        docker exec -it adapter-dev bash
    else
	docker kill adapter-dev
	docker rm adapter-dev

        docker run -d --name adapter-dev --rm -w /adapter/src -v $(pwd)/src:/adapter/src -v $(pwd)/docker/config:/adapter/config -v $(pwd)/docker/scripts/servertest.py:/adapter/scripts/servertest.py -p 1830:830 -p 1222:22 adapter-dev
        if [ ! $? -eq 0 ]; then
            exit 1
        fi
    fi
fi
if $START_ADAPTER; then
    if $GO; then
        docker exec -it adapter-gnb bash
    else
	docker kill adapter-gnb
	docker rm adapter-gnb

        docker run -d --name adapter-gnb -p 11830:830 -p 11221:21 adapter-gnb
        if [ ! $? -eq 0 ]; then
                exit 1
        fi

        docker logs adapter-gnb -f
    fi
fi
