from setuptools import setup, find_packages

def readme():
    with open('README.md') as f:
        return f.read()

setup(
    name='wishful_module_iris',
    version='0.2.0',
    packages=find_packages(),
    url='http://www.wishful-project.eu/software',
    license='',
    author='Piotr Gawlowicz',
    author_email='gawlowicz@tu-berlin.de',
    description='WiSHFUL Module IRIS',
    long_description='WiSHFUL Module IRIS',
    keywords='IRIS SDR platform',
    install_requires=[]
)
