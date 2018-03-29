from os import listdir, environ, path, chdir
import re
import subprocess
import pickle
from time import time
from math import sqrt


skip_benchmarks = ["api_call.wren", "api_foreign_method.wren"]
benchmark_location = 'test/benchmark/'


def separate_class(content, idx):
    in_class = False
    classes = []

    ret_func = []
    ret_class = []
    ret_globals = []

    for line in content.splitlines():
        # handle conflicting class names
        if not in_class and line.startswith("class "):
            in_class = True
            classes.append(line.split(' ')[1])

        to_add = line
        for c in classes:
            rgx = re.compile("[^a-zA-Z]{}[^a-zA-Z]".format(c))
            column = 0
            while True:
                match = rgx.search(to_add, column)
                if not match:
                    break

                begin, end = match.span()
                to_add = "{}{}{}".format(to_add[:begin + 1], c + str(idx), to_add[end - 1:])
                column = end
        
        # handle not global vars that should be
        if not in_class:
            stripped = to_add.strip().split(" ")
            if stripped[0] == 'var' and stripped[1][0].isupper():
                ret_globals.append(stripped[1])
                to_add = to_add.replace('var ', '')

        if in_class:
            ret_class.append(to_add)
        else:
            ret_func.append(to_add)

        if in_class and line.startswith("}"):
            in_class = False
    
    return ret_class, ret_func, ret_globals


def list_files():
    to_ret = []
    for f in listdir(benchmark_location):
        if f.endswith(".wren") and f not in skip_benchmarks:
            to_ret.append(f)
    return to_ret


def compile_combined_benchmark(files):
    contents = []
    idx = 0
    global_vars = []
    for f in files:
        with open(benchmark_location + f, 'r') as fp:
            content_class, content_func, glb = separate_class(fp.read(), idx)
            global_vars.extend(glb)

            header = "// BEGIN {}".format(f)
            root = "\n".join(content_class)
            begin = "var t__{} = Fn.new {{\n\tSystem.print(\" ** Start {}\")".format(idx, f)
            mid = "\n".join("\t{}".format(line) for line in content_func)
            end = "\tSystem.print(\" ** End {}\")\n}}".format(f)
            footer = "// END {}".format(f)

            contents.append("\n\n".join((header, root, begin, mid, end, footer)))

        idx += 1

    to_ret = "\n".join("var " + g for g in global_vars)
    to_ret += "\n"
    to_ret += "\n\n".join(c for c in contents)

    return to_ret
        

def write_out_benchmark(filename, content, files, files_to_skip, iterations):
    clock = "pgo_clock_"
    meas = "measurements_"

    with open(filename, "w") as fp:
        fp.write(content)
        fp.write("\nvar {}\nvar {}\n".format(clock, meas))
        for i, filename in enumerate(files):
            if filename in files_to_skip:
                continue
            fp.write("{} = []\n".format(meas))
            fp.write("for (i in 0..{}) {{\n".format(iterations))
            fp.write("\t{} = System.clock\n".format(clock))
            fp.write("\tt__{}.call()\n".format(i))
            fp.write("\t{}.add(System.clock - {})\n".format(meas, clock))
            fp.write("}\n")
            fp.write("System.write(\"_PGO_ {} \")\n".format(filename))
            fp.write("System.print({})\n".format(meas))


def check_call(cmd, env={}):
    print("Running {} with env {} ...".format(cmd, env), end=" ", flush=True)
    env = {**environ.copy(), **env}

    t = time()
    ps = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if ps.wait() != 0:
        print(ps.communicate()[1].decode())
        raise ValueError("Command returned non zero!")

    print("{:.4f}".format(time() - t))
    return ps.communicate()[0]


def touch_all():
    check_call(["find", "-type", "f", "-exec", "touch", "{}", "+"])


def make_pgo(filename, more_cflags=""):
    check_call(["make", "clean"], {})
    check_call(["make"], {'WREN_CFLAGS': '-fprofile-generate ' + more_cflags})
    check_call(["./wren", filename])
    touch_all()
    check_call(["make"], {'WREN_CFLAGS': '-fprofile-use -w ' + more_cflags})


def make_no_pgo(more_cflags=""):
    check_call(["make", "clean"], {})
    check_call(["make"], {'WREN_CFLAGS': more_cflags})


def measure(filename, cmd="./wren"):
    out = check_call([cmd, filename]).decode()
    to_ret = []
    for line in out.splitlines():
        if "_PGO_" in line:
            splt = line.split(" ")
            data = eval("".join(splt[2:]))

            mean = sum(data) / len(data)
            stddev = sqrt(sum((x - mean) ** 2 for x in data) / (len(data) - 1))

            to_ret.append((mean, stddev))

    return to_ret


if __name__ == "__main__":
    parent_path = "/".join(path.realpath(__file__).split("/")[:-2])
    chdir(parent_path)

    bench_make = "_pgo_make.wren"
    bench_measure = "_pgo_measure.wren"
    results_file = "_pgo_results.p"

    MAKE_ITERS = 4
    MEASURE_ITERS = 40

    data = {}
    files = list_files()
    half = len(files) // 2

    if False:
        content = compile_combined_benchmark(files)
        write_out_benchmark(bench_make, content, files, [], MAKE_ITERS)
        write_out_benchmark(bench_measure, content, files, [], MEASURE_ITERS)

        print("1 / 7")
        make_no_pgo()
        data["release"] = measure(bench_measure)

        print("2 / 7")
        make_no_pgo("-flto")
        data["release-lto"] = measure(bench_measure)
        
        print("3 / 7")
        make_pgo(bench_make)
        data["pgo"] = measure(bench_measure)

        print("4 / 7")
        make_pgo(bench_make, "-flto")
        data["pgo-lto"] = measure(bench_measure)

        print("5 / 7")
        data["system"] = measure(bench_measure, cmd="wren")

        print("6 / 7")
        write_out_benchmark(bench_make, content, files, files[:half], MAKE_ITERS)
        make_pgo(bench_make)
        data["1/2"] = measure(bench_measure)

        print("7 / 7")
        write_out_benchmark(bench_make, content, files, files[half:], MAKE_ITERS)
        make_pgo(bench_make)
        data["2/2"] = measure(bench_measure)

        with open(results_file, "wb") as fp:
            pickle.dump(data, fp)

    else:
        with open(results_file, "rb") as fp:
            data = pickle.load(fp)

    print("*** Results ***")
    fmt = "{:18}"

    # header
    print(fmt.format("*"), end=" ")
    for f in files:
        print(fmt.format(f[:-5]), end=" ")
    print(fmt.format("AVG"))

    # cells
    for data_name, meas in data.items():
        mean_sum = 0
        variance_sum = 0
        print(fmt.format(data_name), end=" ")
        for m in meas:
            strm = "{:.5f}±{:.5f}".format(*m)
            mean_sum += m[0]
            variance_sum += m[1] * m[1]
            print(fmt.format(strm), end=" ")

        stddev = sqrt(variance_sum) / len(meas)
        print("{:.5f}±{:.5f}".format(mean_sum / len(meas), stddev))


