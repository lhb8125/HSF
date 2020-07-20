module utility
  use,intrinsic::iso_c_binding
  implicit none

  integer(kind=4),parameter:: dpL =  kind(.true.)
#ifdef LABEL_INT32
  integer(kind=4),parameter:: dpR =  4
  integer(kind=4),parameter:: dpI =  4
#else
  integer(kind=4),parameter:: dpR =  8
  integer(kind=4),parameter:: dpI =  8
#endif

  include 'mpif.h'
  integer(kind = 4) COMM_SUM
  parameter(COMM_SUM = MPI_SUM)
  integer(kind = 4) COMM_MIN
  parameter(COMM_MIN = MPI_MIN)
  integer(kind = 4) COMM_MAX
  parameter(COMM_MAX = MPI_MAX)
  integer(kind = 4) COMM_PROD
  parameter(COMM_PROD = MPI_PROD)
  integer(kind = 4) COMM_LAND
  parameter(COMM_LAND = MPI_LAND)
  integer(kind = 4) COMM_LOR
  parameter(COMM_LOR = MPI_LOR)
  integer(kind = 4) COMM_BAND
  parameter(COMM_BAND = MPI_BAND)
  integer(kind = 4) COMM_BOR
  parameter(COMM_BOR = MPI_BOR)
  integer(kind = 4) COMM_MAXLOC
  parameter(COMM_MAXLOC = MPI_MAXLOC)
  integer(kind = 4) COMM_MINLOC
  parameter(COMM_MINLOC = MPI_MINLOC)
  integer(kind = 4) COMM_LXOR
  parameter(COMM_LXOR = MPI_LXOR)
  integer(kind = 4) COMM_BXOR
  parameter(COMM_BXOR = MPI_BXOR)
  integer(kind = 4) COMM_REPLACE 
  parameter(COMM_REPLACE = MPI_REPLACE)
  
  ! buffer to hold contents
  character(4096) :: iobuf = ' '

  interface

    subroutine initUtility() bind(c,name='initUtility')
      import
    end subroutine

    subroutine getPid(pid) bind(c,name='getPid')
      import
      integer(c_int):: pid
    end subroutine

    subroutine getCommsize(commSize) bind(c,name='getCommsize')
      import
      integer(c_int):: commSize
    end subroutine

    subroutine allReduceLabels(data, count, op) bind(c,name='allReduceLabels')
      import
      integer(dpI):: data(*)
      integer(dpI), VALUE:: count
      integer(c_int32_T), VALUE:: op
    end subroutine

    subroutine allReduceScalars(data, count, op) bind(c,name='allReduceScalars')
      import
      real(dpR):: data(*)
      integer(dpI), VALUE:: count
      integer(c_int32_T), VALUE:: op
    end subroutine

    subroutine bcastLabels(data, count) bind(c,name='bcastLabels')
      import
      integer(dpI):: data(*)
      integer(dpI), VALUE:: count
    end subroutine

    subroutine bcastScalars(data, count) bind(c,name='bcastScalars')
      import
      real(dpR):: data(*)
      integer(dpI), VALUE:: count
    end subroutine

    subroutine gatherLabels(sdata, rdata, count) bind(c,name='gatherLabels')
      import
      integer(dpI):: sdata(*), rdata(*)
      integer(dpI), VALUE:: count
    end subroutine

    subroutine gatherScalars(sdata, rdata, count) bind(c,name='gatherScalars')
      import
      real(dpR):: sdata(*), rdata(*)
      integer(dpI), VALUE:: count
    end subroutine

    subroutine extremeLabelsInProcs(flag, data, result, count) bind(c,name='extremeLabelsInProcs')
      import
      character(len=1):: flag(*)
      integer(dpR):: data, result
      integer(dpI), VALUE:: count
    end subroutine

    subroutine extremeScalarsInProcs(flag, data, result, count) bind(c,name='extremeScalarsInProcs')
      import
      character(len=1):: flag(*)
      real(dpR):: data, result
      integer(dpI), VALUE:: count
    end subroutine

  end interface

end module