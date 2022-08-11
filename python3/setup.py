import setuptools
from codecs import open as codec_open
from os import close, unlink
from os.path import abspath, dirname, join as joinpath
from re import split as resplit, search as research

META_PATH = joinpath('obl', '__init__.py')

HERE = abspath(dirname(__file__))


def read(*parts):
    """
    Build an absolute path from *parts* and and return the contents of the
    resulting file.  Assume UTF-8 encoding.
    """
    with codec_open(joinpath(HERE, *parts), 'rb', 'utf-8') as dfp:
        return dfp.read()


def read_desc(*parts):
    """Read and filter long description
    """
    text = read(*parts)
    text = resplit(r'\.\.\sEOT', text)[0]
    return text


META_FILE = read(META_PATH)


def find_meta(meta):
    """
    Extract __*meta*__ from META_FILE.
    """
    meta_match = research(
        r"(?m)^__{meta}__ = ['\"]([^'\"]*)['\"]".format(meta=meta),
        META_FILE
    )
    if meta_match:
        return meta_match.group(1)
    raise RuntimeError("Unable to find __{meta}__ string.".format(meta=meta))


setuptools.setup(
    name="obl",
    version=find_meta('version'),
    author="Sebastien Riou",
    author_email="sebastien.riou@tiempo-secure.com",
    description="Library to interact with OBL from host side",
    long_description = """
    Library to interact with OBL from host side
    """,
    long_description_content_type="text/markdown",
    url="https://github.com/sebastien-riou/OBL",
    packages=setuptools.find_packages(),
    data_files = [
        #('jtaghub/dist/windows/64/pe-x86-64',['libjtaghub/dist/windows/64/pe-x86-64/jtaghub64.dll']),
        #('lib/libsstap/dist/linux/64/pe-x86-64'  ,['dependencies/libsstap/libsstap/dist/linux/64/pe-x86-64/sstap64.dll']),
    ],
    #package_data={'': ['libsstap/dist/linux/64/pe-x86-64/sstap64.dll']},
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    include_package_data=True,
    install_requires=[
        'pysatl >= 1.2.5'
        ],
)
