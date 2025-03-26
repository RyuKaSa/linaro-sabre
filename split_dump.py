
dump_location = "dump.md"

output_path = "30fps/test"

index = 0

with open(dump_location, "r") as input:
    data = input.readlines()

    print(data)

    accumulator = []
    for line in data:
        if line == "\n":
            if len(accumulator)>0:
                with open(f"{output_path}/{index}.txt", "w") as output:
                    output.writelines(accumulator)
                    # output.write('\n')
                accumulator = []
                index+=1
        else:
            accumulator.append(line)
            print("adding line #", line, "#")
