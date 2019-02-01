from setuptools import setup

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
      zip_safe=False)


