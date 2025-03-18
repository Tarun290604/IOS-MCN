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


DIRBIN="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DIR_AVAILABLE_YANGS="$DIRBIN/available-yangs"
DIR_YANGS="$DIRBIN/yang"
FILE_DOWNLOADZIP="$DIRBIN/yangs.zip"
FILE_DOWNLOADZIP_RU_ORAN="$DIRBIN/ru-yangs.zip"
FILE_DOWNLOADZIP_DU_ORAN="$DIRBIN/du-yangs.zip"
TMPFOLDER="/tmp/o1-yangs"
UNZIP=$(which unzip)
if [ -z "$UNZIP" ]; then
    echo "unable to find unzip. please install."
    exit 1
fi

# download
if [ ! -f "$FILE_DOWNLOADZIP" ]; then
    # use latest working tag Tag_Rel18_SA103
    wget --no-check-certificate -O "$FILE_DOWNLOADZIP" "https://forge.3gpp.org/rep/sa5/MnS/-/archive/Tag_Rel18_SA103/MnS-Rel-18.zip?path=yang-models"

    # To download RU specific o-ran yangs O-RAN Management Plane Specification - YANG Models 15.0
    wget --no-check-certificate -O "$FILE_DOWNLOADZIP_RU_ORAN" "https://specifications.o-ran.org/download?id=662"

    # To download DU specific o-ran yangs O-RAN O1 Interface specification for O-DU - YANG Models 9.0
    wget --no-check-certificate -O "$FILE_DOWNLOADZIP_DU_ORAN" "https://specifications.o-ran.org/download?id=593"
fi
if [ ! -d "$DIR_AVAILABLE_YANGS" ]; then
    mkdir "$DIR_AVAILABLE_YANGS"
fi
# cleanup yang folders
rm -rf "$DIR_AVAILABLE_YANGS/*"
rm -rf "$DIR_YANGS/*"
unzip -uj yangs.zip -d "$TMPFOLDER"
unzip -uoj du-yangs.zip -d "$TMPFOLDER"
unzip -uoj ru-yangs.zip -d "$TMPFOLDER"
cp -r "$TMPFOLDER/"* "$DIR_AVAILABLE_YANGS/"

rm "$FILE_DOWNLOADZIP"
rm "$FILE_DOWNLOADZIP_RU_ORAN"
rm "$FILE_DOWNLOADZIP_DU_ORAN"

# fill yang folder
cp "$DIR_AVAILABLE_YANGS/"_3gpp-common*.yang "$DIR_YANGS"
cp "$DIR_AVAILABLE_YANGS/"ietf-*.yang "$DIR_YANGS/"
cp "$DIR_AVAILABLE_YANGS/"iana-*.yang "$DIR_YANGS/"

declare special_files=(
    "_3gpp-5g-common-yang-types.yang"
    "_3gpp-5gc-nrm-configurable5qiset.yang"
    "_3gpp-5gc-nrm-ecmconnectioninfo.yang"
    "_3gpp-nr-nrm-bwp.yang"
    "_3gpp-nr-nrm-ep.yang"
    "_3gpp-nr-nrm-gnbcucpfunction.yang"
    "_3gpp-nr-nrm-gnbcuupfunction.yang"
    "_3gpp-nr-nrm-gnbdufunction.yang"
    "_3gpp-nr-nrm-nrsectorcarrier.yang"
    "_3gpp-nr-nrm-nrcelldu.yang"
    "o-ran-interfaces.yang"
    "o-ran-processing-element.yang"
    "o-ran-compression-factors.yang"
    "o-ran-module-cap.yang"
    "o-ran-hardware.yang"
    "o-ran-common-yang-types.yang"
    "o-ran-delay-management.yang"
    "o-ran-uplane-conf.yang"
    "o-ran-wg4-features.yang"
    "o-ran-usermgmt.yang"
    "o-ran-operations.yang"
    "o-ran-aggregation-base.yang"
    "o-ran-agg-uplane-conf.yang"
)

for file in "${special_files[@]}"
do
   cp "$DIR_AVAILABLE_YANGS/$file" "$DIR_YANGS/"
done
