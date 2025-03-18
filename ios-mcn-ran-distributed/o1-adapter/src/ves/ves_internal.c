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

#define _GNU_SOURCE

#include "ves_internal.h"
#include "common/utils.h"
#include "common/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

struct memory {
  char *response;
  size_t size;
};

static int ves_http_request(const char *url,
                            const char *username,
                            const char *password,
                            const char *method,
                            const char *send_data,
                            int *response_code,
                            char **recv_data);
static int ves_dummy_http_request(const char *url,
                                  const char *username,
                                  const char *password,
                                  const char *method,
                                  const char *send_data,
                                  int *response_code,
                                  char **recv_data);
static size_t curl_write_cb(void *data, size_t size, size_t nmemb, void *userp);

const config_t *ves_config = 0;
ves_common_header_t ves_common_header = {0};

int ves_execute(const char *content, const char *domain, const char *event_type, const char *priority)
{
  char timestampMicrosec[32];
  char *timestampISO3milisec;
  char seqId[32];

  timestampISO3milisec = get_netconf_timestamp_with_miliseconds(0);
  sprintf(timestampMicrosec, "%lu", get_microseconds_since_epoch());
  sprintf(seqId, "%d", ves_common_header.seq_id);

  char *post_data = strdup(content);
  if (post_data == 0) {
    log_error("strdup failed");
    goto failed;
  }

  // common event header replacements - method params
  post_data = str_replace_inplace(post_data, "@domain@", domain);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@eventType@", event_type);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@priority@", priority);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  // common event header replacements - ves_common_header
  post_data = str_replace_inplace(post_data, "@seqId@", seqId);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@managed-element-id@", ves_common_header.info.managed_element_id);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@node-id@", ves_config->info.node_id);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@vendor@", ves_common_header.info.vendor);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  // other common replacements
  post_data = str_replace_inplace(post_data, "@timestampMicrosec@", timestampMicrosec);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  post_data = str_replace_inplace(post_data, "@timestampISO3milisec@", timestampISO3milisec);
  if (post_data == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  // send request
  int response_code;
  char *response = 0;
  int rc;
  rc = ves_http_request(ves_config->ves.url,
                        ves_config->ves.username,
                        ves_config->ves.password,
                        "POST",
                        post_data,
                        &response_code,
                        &response);
  // rc = ves_dummy_http_request(ves_config->ves.url, ves_config->ves.username, ves_config->ves.password, "POST", post_data,
  // &response_code, &response);
  if (rc != 0) {
    log_error("ves_http_request() failed");
    goto failed;
  }

  log("response_code = %d", response_code);
  if (response) {
    log("response = %s", response);
  }

  free(response);
  if (response_code > 399) {
    log_error("failure http response code: %d", response_code);
    goto failed;
  }

  ves_common_header.seq_id++;
  free(post_data);
  free(timestampISO3milisec);

  return 0;

failed:
  free(post_data);
  free(timestampISO3milisec);

  return 1;
}

int ves_vsftp_daemon_init(void)
{
  return system("/usr/sbin/vsftpd &");
}

int ves_vsftp_daemon_deinit(void)
{
  return system("killall -9 vsftpd");
}

int ves_sftp_daemon_init(void)
{
  return system("/usr/sbin/sshd -D &");
}

int ves_sftp_daemon_deinit(void)
{
  return system("killall -9 sshd");
}

static int ves_http_request(const char *url,
                            const char *username,
                            const char *password,
                            const char *method,
                            const char *send_data,
                            int *response_code,
                            char **recv_data)
{
  const char *send_data_good = send_data;
  if (send_data_good == 0) {
    send_data_good = "";
  }

  CURL *curl = curl_easy_init();
  if (curl == 0) {
    log_error("curl_easy_init() error");
    goto failed;
  }

  // set curl options
  struct curl_slist *header = 0;
  header = curl_slist_append(header, "Content-Type: application/json");
  if (!header) {
    log_error("curl_slist_append() failed");
    goto failed;
  }

  header = curl_slist_append(header, "Accept: application/json");
  if (!header) {
    log_error("curl_slist_append() failed");
    goto failed;
  }

  header = curl_slist_append(header, "X-MinorVersion: 1");
  if (!header) {
    log_error("curl_slist_append() failed");
    goto failed;
  }

  CURLcode res = CURLE_OK;
  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L); // seconds timeout for a connection
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L); // seconds timeout for an operation
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  // disable SSL verifications
  res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYHOST, 0L);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, send_data_good);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  res = curl_easy_setopt(curl, CURLOPT_URL, url);
  if (res != CURLE_OK) {
    log_error("curl_easy_setopt() error");
    goto failed;
  }

  char *credentials = 0;
  if ((username) && (password)) {
    asprintf(&credentials, "%s:%s", username, password);
    if (credentials == 0) {
      log_error("asprintf() failed");
      goto failed;
    }

    res = curl_easy_setopt(curl, CURLOPT_USERPWD, credentials);
    if (res != CURLE_OK) {
      log_error("curl_easy_setopt() error");
      goto failed;
    }
  }

  struct memory response_data = {0};
  if (recv_data) {
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    if (res != CURLE_OK) {
      log_error("curl_easy_setopt() error");
      goto failed;
    }
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
    if (res != CURLE_OK) {
      log_error("curl_easy_setopt() error");
      goto failed;
    }
  }

  log("%s-ing cURL to url=\"%s\" with body=\"%s\"... ", method, url, send_data_good);
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    log_error("curl_easy_perform() failed");
    goto failed;
  }

  if (response_code) {
    long http_rc;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_rc);
    if (res != CURLE_OK) {
      log_error("curl_easy_getinfo() failed");
      goto failed;
    }
    *response_code = http_rc;
  }

  if (recv_data) {
    *recv_data = response_data.response;
  }

  free(credentials);
  curl_slist_free_all(header);
  curl_easy_cleanup(curl);
  return 0;

failed:
  free(credentials);
  curl_slist_free_all(header);
  curl_easy_cleanup(curl);
  return 1;
}

static int ves_dummy_http_request(const char *url,
                                  const char *username,
                                  const char *password,
                                  const char *method,
                                  const char *send_data,
                                  int *response_code,
                                  char **recv_data)
{
  log("ves_dummy_http_request");
  log("======================");
  log("url: %s", url);
  log("username: %s", username);
  log("password: %s", password);
  log("method: %s", method);
  log("send_data: %s", send_data);
  *response_code = 200;
  asprintf(recv_data, "DUMMY RESPONSE");

  return 0;
}

static size_t curl_write_cb(void *data, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)userp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (ptr == NULL) {
    // TODO log_error("realloc failed\n");
    return 0; /* out of memory! */
  }

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}
