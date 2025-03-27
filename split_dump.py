
dump_location = "dump_sobel_2.md"

output_path = "30fps/sobel_2"

index = 0

with open(dump_location, "r") as input:
    data = input.readlines()

    # print(data)

    accumulator = []
    for line in data:
        if line == "\n":
            if len(accumulator)>0:
                with open(f"{output_path}/{str(index+1).rjust(2, '0')}.txt", "w") as output:
                    output.writelines(accumulator)
                    # output.write('\n')
                accumulator = []
                index+=1
        else:
            accumulator.append(line)
            # print("adding line #", line, "#")

print(f"found {index} dumps in {dump_location} Sent in {output_path}")