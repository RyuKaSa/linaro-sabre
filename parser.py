import os
import re

# filename = "perfs"

base_folder =  ["30fps"]
version_folder = ["baseline", "medium"]

for base in base_folder:

    print(f"Version {base}:")

    all_average = {}

    for version in version_folder:

        input_folder = f"{base}/{version}"

        call_list = {}

        for filename in os.listdir(input_folder):
            with open(f"{input_folder}/{filename}", "r") as f:
                data = list(filter(lambda x: x != "\n", f.readlines()))
                call_list[filename] = []
                # print(filename, len(data))
                for l in data[3:]:
                    dico = {}
                    values = l.split()
                    dico['percent']       = float(values[0])
                    dico['cum sec']       = float(values[1])
                    dico['self sec']      = float(values[2])
                    dico['calls']         = int(values[3])
                    dico['ms/call']       = float(values[4])
                    dico['tt ms/call']    = float(values[5])
                    dico['name']          = values[6]
                    call_list[filename].append(dico)

        all_times = 0
        for v in call_list:
            duration = max([d['cum sec'] for d in call_list[v]])
            all_times += duration
        all_average[version] = all_times/len(call_list)
        print(f"    Average for {version} is {all_average[version]}")

    sorted_times = sorted(all_average.items(), key=lambda x: x[1])
    print(f"The best code so far is {sorted_times[0][0]} with {sorted_times[0][1]} seconds ({30/sorted_times[0][1]}fps)")
