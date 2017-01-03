#!/usr/bin/env python3
import subprocess, shutil, time, sys, os

def run(commands):
    try:
        subprocess.run(commands, check=True)
    except subprocess.CalledProcessError as e:
        print("The command (%s) failed with output:" % commands)
        print(e.output)
        raise

def cleardir(dirname):
    for filename in os.listdir(dirname):
        filepath = os.path.join(dirname, filename)
        if os.path.isfile(filepath):
            os.unlink(filepath)

def benchmark(f, iterations, *args, **kwargs):
    begin  = time.time()
    for i in range(iterations):
        f(*args, **kwargs)
    end    = time.time()
    total  = (end - begin) / iterations
    return total

def build(directory, languageargs):
    inputfiles = []

    for dirpath, dnames, filenames in os.walk(directory):
        for filename in filenames:
            filepath    = os.path.join(dirpath, filename)
            filename, _ = os.path.splitext(filename)
            inputfile   = os.path.join('input/', filename)

            inputfiles.append(filename) # Uses filename, since the compiler knows to use input/output directories
            shutil.copyfile(filepath, inputfile)
            if languageargs[0] in ['python', 'python2', 'python3']:
                run(['./annotate.py', inputfile])

    print('Running files: %s' % '\n'.join(inputfiles))
    run(['./build/progtran'] + languageargs + inputfiles)

def time_run(language, directory, iterations, filename='main'):
    with open('languages/' + language + '/run', 'r') as runfile:
        content = [line.replace('@', filename) for line in runfile]
    olddir = os.getcwd()
    shutil.copytree(directory, directory + '_copy')
    os.chdir(directory + '_copy')
    try:
        for line in content[:-1]:
            subprocess.check_output(line, shell=True) # Yes, this is unescaped.
        t = benchmark(subprocess.check_output, iterations, content[-1], shell=True)
    finally:
        os.chdir(olddir)
        shutil.rmtree(directory + '_copy')
    return t

def compare(directory, inputlang, outputlang, iterations=1):
    # Compile output c++ code (hardcoded for now, since output language is always c++)
    if outputlang == 'cpp':
        os.chdir('output')
        subprocess.run('g++ -std=c++14 *.cpp -Os ../std/*.cpp', shell=True)
        output_time = benchmark(subprocess.check_output, iterations, './a.out', shell=True)
        os.chdir('..')
    else:
        output_time = time_run(outputlang, 'output', iterations)
    print('Output code time:')
    print(output_time)

    # Timing of inputlang isn't hardcoded:
    input_time = time_run(inputlang, directory, iterations)
    print('Input code time:')
    print(input_time)
    print('Transpile speedup:')
    transpile_speedup = input_time / output_time
    print(transpile_speedup)

    '''
    if inputlang in ['python2', 'python3']:
        cythonversion = 'cython3' if inputlang == 'python3' else 'cython'
        cython_time = time_run(cythonversion, directory, iterations)
        print('Cython time:')
        print(cython_time)
        print('Cython speedup:')
        cython_speedup = cython_time / output_time
        print(cython_speedup)
        
        print('Transpile : Cython comparison (1> indicates transpile is faster than cython)')
        print(transpile_speedup / cython_speedup)
    '''

        

    

def main():
    # Build the compiler and test it
    os.chdir('build')
    run(['cmake', '..'])
    run(['make'])
    os.chdir('..')
    run(['./build/progtrantest'])

    # Build list of demonstrations
    with open('examples/demos') as demofile:
        content = [line for line in demofile]

    demos = dict()
    for line in content:
        terms = line.split() 
        assert len(terms) > 1
        a = terms[0]
        demos[a] = ['examples/' + a] + terms[1:]

    # Determine which demo to use
    if len(sys.argv) > 1:
        if sys.argv[1] == '--show': # Display which demos are available if --show flag given
            for line in content:
                print(line, end='')
            sys.exit(0)
        demoname = sys.argv[1]      # Otherwise, use demo provided
    else:
        demoname = 'python3'         # If none provided, show python demo by default

    if not os.path.exists('output'):
        os.makedirs('output')
    if not os.path.exists('input'):
        os.makedirs('input')

    try:
        # Build/compare input/output languages
        l = demos[demoname]
        directory = l[0]
        languageargs = l[1:]
        build(directory, languageargs)
        compare(directory, languageargs[0], languageargs[1], iterations=100)

        # Save demo output, then cleanup
        outputdir = 'examples/output/' + demoname + '_output'
        if os.path.exists(outputdir):
            shutil.rmtree(outputdir)
        shutil.copytree('output', outputdir)
    finally:
        shutil.rmtree('output')
        shutil.rmtree('input')

if __name__ == '__main__':
    main()
