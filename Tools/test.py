import subprocess
import os
import io

os.environ["PYTHONUNBUFFERED"] = "1"

Output = io.StringIO()

cmd = ["ls", "-R", "/Users/danielburnier/EPFL-Mobots/Projects/e-puck/e-puck2/Softwares"]
process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

while process.poll() is None:
    out = process.stdout.read(1)
    if out != '':
        print(out, end='')
        Output.write(out)

# Wait for the subprocess to finish and get its return code
return_code = process.wait()
print(f"Subprocess returned with exit code: {return_code}")
input()
Output.getvalue()
Output.close()


# Tester output = check_output("dmesg | grep hda", shell=True)
