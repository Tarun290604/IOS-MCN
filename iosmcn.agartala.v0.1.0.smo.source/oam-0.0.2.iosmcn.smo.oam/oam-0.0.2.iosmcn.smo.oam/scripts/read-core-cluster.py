#  ============LICENSE_START===============================================
#  Copyright (C) 2024 Linux Foundation. All rights reserved.
#  ========================================================================
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  ============LICENSE_END=================================================
#

from pprint import pprint
from kubernetes import client, config
import requests
from dotenv import set_key
from pathlib import Path


CORE_SERVICES=[
        {'name': 'upf-http','port': 8080, 'ns':'omec', 'key':'CORE_UPF_IP'},
        {'name': 'smf','port': 9089, 'ns':'omec', 'key':'CORE_SMF_IP'},
        {'name': 'metricfunc','port': 9089, 'ns':'omec', 'key':'CORE_METRICFUNC_IP'},
        {'name': 'amf','port': 9089, 'ns':'omec', 'key':'CORE_AMF_IP'},
        {'name': 'rancher-monitoring-prometheus-node-exporter','port': 9796, 'ns':'cattle-monitoring-system', 'key': 'CORE_NODEX_IP'}
        ]

def resources_check(core_api):
    tca = 0
    tma = 0
    node_list = core_api.list_node()
    for node in node_list.items:
        if 'Ki' in node.status.allocatable['memory'][-2:]:
            mvalue = int(node.status.allocatable['memory'][:-2])
            tma = tma + mvalue
        cvalue =int(node.status.allocatable['cpu'])
        tca = tca + cvalue
    print("Total memory allocatable is : " + str(tma) + "Ki")
    print("Total CPU allocatable is : " + str(tca))
        
    
def core_prometheus(core_api, env_file_path):
    urls_to_scrape = []
    for coreservice in CORE_SERVICES:
        service = core_api.read_namespaced_service(name=coreservice['name'], namespace=coreservice['ns'])
        print(coreservice['name'], service.spec.cluster_ip)
        url = "http://" + service.spec.cluster_ip + ":" + str(coreservice['port']) + "/metrics"
        try:
            page = requests.get(url)
            if page.ok:
                set_key(dotenv_path=env_file_path, key_to_set=coreservice['key'], value_to_set=service.spec.cluster_ip)
                urls_to_scrape.append(url)
        except (requests.exceptions.HTTPError, requests.exceptions.ConnectionError):
            print("Error")
    return urls_to_scrape

def apis_supported(api_api, core_api):
    print("%-40s %s" %
          ("core", ",".join(core_api.get_api_versions().versions)))
    for api in api_api.get_api_versions().groups:
        versions = []
        for v in api.versions:
            name = ""
            if v.version == api.preferred_version.version and len(
                    api.versions) > 1:
                name += "*"
            name += v.version
            versions.append(name)
        print("%-40s %s" % (api.name, ",".join(versions)))

def using_token():
    # Define the bearer token we are going to use to authenticate.
    # See here to create the token:
    # https://kubernetes.io/docs/tasks/access-application-cluster/access-cluster/
    aToken = "<token>"

    # Create a configuration object
    aConfiguration = client.Configuration()

    # Specify the endpoint of your Kube cluster
    aConfiguration.host = "https://XXX.XXX.XXX.XXX:443"

    # Security part.
    # In this simple example we are not going to verify the SSL certificate of
    # the remote cluster (for simplicity reason)
    aConfiguration.verify_ssl = False
    # Nevertheless if you want to do it you can with these 2 parameters
    # configuration.verify_ssl=True
    # ssl_ca_cert is the filepath to the file that contains the certificate.
    # configuration.ssl_ca_cert="certificate"
    aConfiguration.api_key = {"authorization": "Bearer " + aToken}

    # Create a ApiClient with our config
    aApiClient = client.ApiClient(aConfiguration)
    # Do calls
    v1 = client.CoreV1Api(aApiClient)

def append_to_conf(prom_urls):
    try:
        with open("../config/telegraf/oam-telegraf.conf", 'a') as file:
            file.write("\n" + "[[inputs.prometheus]]" + '\n')
            file.write("    urls = [\"")
            for url in prom_urls[0:-1]:
                file.write(url)
                file.write("\"" + "," + "\"")
            file.write(prom_urls[-1])
            file.write("\"]" + "\n")
    except Exception as e:
        print(f"Error: {e}")

    

# Other Things to do:
# https://github.com/kubernetes-client/python/blob/05e8f5798aa40be42cc13cfc115c753ca66d089b/examples/dynamic-client/configmap.py 

def main():
    env_file_path = Path("../.corenv")
    env_file_path.touch(mode=0o600, exist_ok=True)
    config.load_kube_config()
    custom_objects_api = client.CustomObjectsApi()
    core_api = client.CoreApi()
    core_v1_api = client.CoreV1Api()
    apps_v1_api = client.AppsV1Api()
    networking_v1_api = client.NetworkingV1Api()
    api_v1_api = client.ApisApi()
    print("********** APIs SUPPORTED **************\n")
    # Check APIs supported
    apis_supported(api_v1_api, core_api)
    print("\n********************************************\n")
    # check resources allocatables
    print("********** ALLOCATABLE RESOURCES **************\n")
    resources_check(core_v1_api)
    print("\n********************************************\n")
    # Get Core Prometheus End-Points
    print("******** PROMETEUS ENDPOINTS TO SCRAPE *******\n")
    prom_urls = core_prometheus(core_v1_api, env_file_path)
    print("\n********************************************\n")
    append_to_conf(prom_urls)

if __name__ == "__main__":
    main()
