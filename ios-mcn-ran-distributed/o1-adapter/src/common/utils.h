/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Copyright: Fraunhofer Heinrich Hertz Institute
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#pragma once

long int get_seconds_since_epoch(void);
long int get_microseconds_since_epoch(void);
char *get_netconf_timestamp(void);
char *get_netconf_timestamp_with_miliseconds(int addSeconds);
int put_human_timestamp(char *nctime);
char *str_replace(const char *orig, const char *rep, const char *with);
char *str_replace_inplace(char *s, const char *rep, const char *with);
char *str_remove_whole_line_inplace(char *haystack, const char *needle);
char *get_hostname(void);
unsigned long int get_file_size(const char *filename);
char *file_read_content(const char *fname);
