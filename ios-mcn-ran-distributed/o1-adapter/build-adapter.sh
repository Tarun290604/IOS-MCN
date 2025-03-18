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

BUILD_DEV="false"
BUILD_ADAPTER="false"

until [ -z "$1" ]
do
    case "$1" in 
        --dev)
            BUILD_DEV="true"
            shift
            ;;

        --adapter)
            BUILD_ADAPTER="true"
            shift
            ;;

        --no-cache)
            NO_CACHE="--no-cache"
            shift
            ;;

        *)
            shift
            ;;
    esac
done

if $BUILD_DEV; then
    docker build $NO_CACHE --target adapter-dev --tag adapter-dev:latest --file docker/Dockerfile.dev .
    if [ ! $? -eq 0 ]; then
        exit 1
    fi
fi
if $BUILD_ADAPTER; then
    docker build $NO_CACHE --target adapter-gnb --tag adapter-gnb:latest --file docker/Dockerfile.adapter .
    if [ ! $? -eq 0 ]; then
            exit 1
    fi
fi
