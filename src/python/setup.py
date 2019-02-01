from setuptools import setup
from setuptools.command import install 
from distutils.cmd import Command
import glob
import os
import shutil

here = os.getcwd()

class CleanCommand(Command):
    """Custom clean command to tidy up the project root."""
    CLEAN_FILES = './build ./dist ./*.pyc ./*.tgz ./*.egg-info'.split(' ')

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        global here

        for path_spec in self.CLEAN_FILES:
            # Make paths absolute and relative to this path
            abs_paths = glob.glob(os.path.normpath(os.path.join(here, path_spec)))
            for path in [str(p) for p in abs_paths]:
                if not path.startswith(here):
                    # Die if path in CLEAN_FILES is absolute + outside this directory
                    raise ValueError("%s is not a path inside %s" % (path, here))
                print('removing %s' % os.path.relpath(path))
                shutil.rmtree(path)




setup(name='hallc',
      version='0.1',
      description='Hall C python package',
      url='http://github.com/whit2333/hallc_tools',
      author='Whitney Armstrong',
      author_email='whit@jlab.org',
      license='MIT',
      packages=['hallc'],
      #install_requires=[ 'markdown', ],
      #dependency_links=['http://github.com/user/repo/tarball/master#egg=package-1.0']
      test_suite='nose.collector',
      tests_require=['nose'],
      scripts=['bin/debug_run_info'],
      include_package_data=True,
      classifiers=[
          "Programming Language :: Python :: 3",
          "License :: OSI Approved :: MIT License",
          "Operating System :: OS Independent",
          ],
      zip_safe=False,
      cmdclass = {'clean':CleanCommand}
      )
