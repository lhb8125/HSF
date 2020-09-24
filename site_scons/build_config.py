PLATFORM = 'sw'
INT_TYPE = '64'
FLOAT_TYPE = '64'
LIB_TYPE = 'static'

ATHREAD = 'True'
PRINT_VERBOSE = 'False'

# sw
# CXX = 'swg++'
# CC = 'swgcc'
# F90 = 'swgfortran453'
# CXX_LINKER = 'mpiCC'
# F_LINKER = 'mpiswgfortran'

# x86 gcc
# CXX = 'mpicxx'
# CC = 'mpicc'
# F90 = 'mpif90'
# CXX_LINKER = 'mpicxx'
# F_LINKER = 'mpif90'

# MPI_INC_PATH = 'C:\Program Files\MPICH2\include'
# MPI_LIB_PATH = 'C:\Program Files\MPICH2\lib'

# MPI_INC_PATH = 'C:\Program Files (x86)\Microsoft SDKs\MPI\Include'
# MPI_LIB_PATH = 'C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\\x64'

THIRDPARTY_INCS = []
THIRDPARTY_LIBS = []
THIRDPARTY_LIB_PATH = []
software_home = '/home/export/online3/amd_dev1/software'
if PLATFORM == 'sw':
    yaml_home = software_home + '/yaml-cpp/yaml-cpp-0.5.3'
    cgns_home = software_home + '/CGNS/CGNS-3.4.0/sw_install'
    hdf5_home = software_home + '/HDF5/hdf5-1.8.8'
    # hdf5_home = '/home/export/online3/amd_dev1/liuhb/software/hdf5-1.8.8'
    boost_home = software_home + '/boost/boost1.69'
    unat_home = '/home/export/online3/amd_dev1/liuhb/unat/install'
    utility_home = '/home/export/online3/amd_dev1/guhf/utilities/install/swswg++Int64Float64Opt'
    parmetis_home = software_home + '/ParMETIS/parmetis-4.0.3/SWInstall_64_64'

    THIRDPARTY_INCS = [
        yaml_home + '/include', cgns_home + '/include', hdf5_home + '/include',
        parmetis_home + '/include', boost_home + '/include',
        unat_home + '/include', utility_home + '/include'
    ]

    THIRDPARTY_LIB_PATH = [
        yaml_home + '/lib', cgns_home + '/lib', hdf5_home + '/lib',
        parmetis_home + '/lib', unat_home + '/lib', utility_home + '/lib'
    ]

    THIRDPARTY_LIBS = [
        'hsf', 'UNAT', 'utilities', 'cgns', 'hdf5', 'parmetis', 'metis',
        'yaml-cpp', 'm', 'z', 'dl', 'stdc++'
    ]

if PLATFORM == 'linux':
    yaml_home = software_home + '/yaml-cpp/yaml-cpp-0.6.2'
    cgns_home = software_home + '/CGNS/CGNS-3.4.0/gcc_install'
    hdf5_home = software_home + '/HDF5/hdf5-1.10.5/gcc_build'
    parmetis_home = software_home + '/ParMETIS/parmetis-4.0.3/gccInstall_64_64'
    utility_home = '/home/export/online3/amd_dev1/guhf/utilities/install/linuxmpicxxInt64Float64Opt'

    THIRDPARTY_INCS = [
        yaml_home + '/include', cgns_home + '/include', hdf5_home + '/include',
        parmetis_home + '/include', utility_home + '/include'
    ]

    THIRDPARTY_LIB_PATH = [
        yaml_home + '/lib', cgns_home + '/lib', hdf5_home + '/lib',
        parmetis_home + '/lib', utility_home + '/lib'
    ]

    THIRDPARTY_LIBS = [
        'hsf', 'utilities', 'cgns', 'hdf5', 'parmetis', 'metis', 'yaml-cpp',
        'm', 'z', 'dl', 'stdc++'
    ]
