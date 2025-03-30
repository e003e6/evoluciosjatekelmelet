from distutils.core import setup, Extension

pyheader = ''
npheader = "/Users/klapkazsombor/fileok/venvs/jupyter_venvs/adatos_3-10-11/lib/python3.10/site-packages/numpy/core/include"

modul = Extension("cmodul", sources=['pyinterface.c', 'szimulacio.c'], include_dirs=[npheader])

setup(name='cmodul', ext_modules=[modul])


