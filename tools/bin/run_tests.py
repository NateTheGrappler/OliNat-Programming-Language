from pathlib import Path
import subprocess
import difflib
import sys

#array holding all of the test case file paths
testFiles = []
TIMEOUT_SECONDS = 10 #time for a test to hang before failing

#an extra argument to run into the command line, basically just builds the expected and exit code files for you
UPDATE_MODE = "--update" in sys.argv

#make sure that the path for running the file is the same no matter where it runs from
SCRIPT_DIR = Path(__file__).parent
testCases_path = (SCRIPT_DIR / "../../test_cases").resolve()
INTERPRETER_PATH = (SCRIPT_DIR / "../../cmake-build/Oli_Nat").resolve()

#loop through all of the .oli files in the test_cases directory
for file in testCases_path.rglob("*"):
    #recursively search through all directories for test files only
    if file.suffix == ".oli":
        testFiles.append(file)

#quick little null check for the test files array
if not testFiles:
    sys.exit(f"No .oli tests found in {testCases_path}")

#sort the testcases so they are always in order
testFiles.sort()
#print(testFiles)

def run_program(source):

    #run each of the .oli files into the executable that gets built in cmake
    try:     
        result = subprocess.run(
            [INTERPRETER_PATH, str(source)],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SECONDS
        )
    except subprocess.TimeoutExpired:
        #test to make sure that something does not hang forever
        return None, "", f"Test timed out after {TIMEOUT_SECONDS}s"

    #return the 0, 65, or 70 return codes so you can test errors, as well as output, and then stderr
    return result.returncode, result.stdout, result.stderr


#print(run_program(testFiles[0]))


def checkTest(source):
    #look for the file with the same name as the source code .oli file, but just this time with what the expected output should be
    expected_file = source.with_suffix(".expected")
    exitcode_file = source.with_suffix(".exitcode")

    #run the actual test file through the executable
    returncode, output, error = run_program(source)

    #make sure that the test file did not time out
    if(returncode is None): return "fail", error


    #make sure nobody tries to commit a seg fault to the interpreter
    if returncode < 0:
        return "fail", f"interpreter crashed (signal {-returncode})\n{error}"

    #if the update flag was set then build the expected files for the tests
    if(UPDATE_MODE):
        expected_file.write_text(output)

        #if there is some weird output code then add that as the expected output code, really
        #here for the sake of me not writting a million files, but can be bad because this can
        #just like ignore a segfault if the person writting the test cases is a dumbass (I may be a dumbass)
        if(returncode != 0):
            exitcode_file.write_text(f"{returncode}\n")
        elif exitcode_file.exists():
            exitcode_file.unlink() #delete the file if it just outputs zero because you dont need it

        return ("updated", "")

    #if file does not exist, fail this "test" and then also see if you can eventually manually update one
    if not expected_file.exists():
        return "fail", f"missing {expected_file.name}"

    #array of issues with tests to print out for the user
    problems = []


    #check to see if there is an exitcode file, start at 0 for successful return:
    expected_code = 0
    if(exitcode_file.exists()):
        #convert the expected exit code in the file from a string to an int
        expected_code = int(exitcode_file.read_text().strip())


    #compare the exitcode from the interpreter with the one in the .exitcode file
    if(returncode != expected_code):
        problems.append(f"exit code {returncode}, expected {expected_code}")
        #if the interpreter crashes, like a segfault, then print that out
        if(error.strip()):
            problems.append("stderr:\n" + error.rstrip())

    #compare the text output from the interpreter with the one from the file
    expected = expected_file.read_text()
    if(expected != output):

        #use the fancy github like difference to show what is different
        diff = difflib.unified_diff(
            expected.splitlines(keepends=True),
            output.splitlines(keepends=True),
            fromfile="expected",
            tofile="actual",
        )
        problems.append("output differs:\n" + "".join(diff).rstrip())


    #if you had no problems found, then return you passed, otherwise print out problems found
    if(problems):
        return ("fail", "\n".join(problems))
    else:
        return ("pass", "")


passed = 0
failed = 0


#loop over all of the files and run them while capturing their output
for file in testFiles:

    #get if the test passed and then also the message given, then also print out the file name
    status, message = checkTest(file)

    if status == "pass":
        passed += 1
        print("PASS", file.name)
    elif status == "updated":
        print("UPDATED", file.name)
    else:
        failed += 1
        print("FAIL", file.name)
        print(message)


#summary of the whole run, how many failed and how manny passed
print(f"\n{passed} passed, {failed} failed")


if(failed == 0):
    sys.exit(0)
else:
    sys.exit(1)

