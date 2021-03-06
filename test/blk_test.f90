!-------------------------------------------------------------------------------
!   module defines precisions of logical, integer and real.
!-------------------------------------------------------------------------------
module var_kind_def
    implicit none

    integer(kind=4),parameter:: dpL =  kind(.true.)
    integer(kind=4),parameter:: dpI =  8
    integer(kind=4),parameter:: dpR =  8
end module var_kind_def

module var_global
    use var_kind_def
    implicit none
    integer, parameter:: strlen = 100
    integer, parameter:: dim = 3
    integer, parameter:: max_n_node = 50
    integer(dpI):: err_mem = 0
    integer:: my_id, ierr

end module var_global

module var_c_string
    use var_kind_def
    use iso_c_binding
    implicit none
    ! integer, parameter:: str_num = 10
    character(len=30):: str_arr(1)
    POINTER(str_ptr, str_arr)
end module

program main
    use var_kind_def
    use var_global
    use utility
    use var_c_string
    use iso_c_binding

    implicit none

    type,bind(c)::struct_scalarField
      integer(c_long):: locSize
      integer(c_long):: nbrSize
      integer(c_long):: ndim
      type(c_ptr):: data
    end type

    interface
      type(struct_scalarField) function get_scalar_field(name1, name2) bind(c,name='get_scalar_field_')
        import !! Make iso_c_binding and struct_scalarField visible here
        ! character(len=20):: name1, name2
        character (kind=c_char, len=1), dimension (*), intent (in) :: name1, name2
      end function
    end interface

    character(len=strlen):: config_file = './config.yaml'
    integer(dpI):: iele, iface, inode
    integer(dpI):: n_ele, n_face_i, n_face, n_face_b, n_node, face_start, face_end, n_nbr_ele
    integer(dpI),allocatable:: e2n(:),e2n_pos(:),ele_type(:)
    integer(dpI),allocatable:: e2f(:),e2f_pos(:)
    integer(dpI),allocatable:: if2n(:),if2n_pos(:),bf2n(:)
    integer(dpI),allocatable:: if2e(:),if2e_pos(:),bf2e(:)
    real(dpR),allocatable:: b(:),x(:),A(:)
    real(dpR), allocatable:: vol(:), area(:), coord(:)
    real(dpR),allocatable:: pid(:), id(:)
    character(len=20):: field_name, field_type
    real(c_double), pointer:: b_new(:), x_new(:), A_new(:)
    type(struct_scalarField):: b_tmp, x_tmp, A_tmp

    real(dpR) :: vol_new(1)
    POINTER(vol_new2, vol_new)
    integer(dpI):: ndim_new, n_ele_new, n_dim

    integer:: nPara, write_interval, str_len
    character(20):: mesh_file, result_file
    real:: delta_t

    integer(dpI):: n_blk, iblk, ele_nodes, nodes_num_for_ele
    integer(dpI):: faces_num_for_ele, ele_faces, face_nodes
    integer(dpI),allocatable:: eblkt(:), fblkt(:), eblkS(:), fblkS(:), bnd_type(:)

    integer(dpI):: local_val, max_val, min_val, count
    real(dpR):: local_real, min_real

    ! procedure(func),pointer:: f_ptr => null()

    ! 初始化
    call init_utility_fort()
    call init(trim(config_file))

    ! 获取网格文件名
    nPara = 4
    call get_string_para(mesh_file, str_len, nPara, &
        & "domain1"//C_NULL_CHAR, &
        & "region"//C_NULL_CHAR, &
        & "0"//C_NULL_CHAR, &
        & "path"//C_NULL_CHAR)

    ! ! 获取基本单元数目
    call get_elements_num(n_ele)
    call get_inner_faces_num(n_face_i)
    call get_faces_num(n_face)
    call get_bnd_faces_num(n_face_b)
    call get_nodes_num(n_node)
    call get_ele_blk_num(n_blk)
    write(*,*), "elements num: ", n_ele
    write(*,*), "internal faces num: ", n_face_i
    write(*,*), "processors faces num: ", n_face - n_face_i
    write(*,*), "boundary faces num: ", n_face_b
    write(*,*), "nodes num: ", n_node
    write(*,*), "cell blocks num: ", n_blk

    ! 获取网格单元类型数目及起始位置
    allocate(eblkt(n_blk), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, eblkt'
    call get_ele_blk_type(eblkt)
    allocate(eblkS(n_blk+1), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, eblkS'
    call get_ele_blk_pos(eblkS)
    do iblk=1,n_blk
        write(*,*), "The type of ", iblk, "th block: ", eblkt(iblk)
        write(*,*), "start from ", eblkS(iblk), ", end at ", eblkS(iblk+1)-1
    end do

    ! 计算网格单元所含格点总数
    ele_nodes = 0
    do iblk=1,n_blk
        ele_nodes = ele_nodes + nodes_num_for_ele(eblkt(iblk))*(eblkS(iblk+1)-eblkS(iblk))
    end do
    write(*,*), "nodes num in elements: ", ele_nodes

    ! 获取进程号
    call get_pid(my_id)
    allocate(pid(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, pid'
    do iele=1,n_ele
        pid(iele) = my_id
    end do
    ! 注册进程号场
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "pid"//C_NULL_CHAR, pid, n_dim)

    ! 获取单元与格点拓扑关系
    allocate(e2n(ele_nodes), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, e2n'
    call get_ele_2_node_blk(e2n)
    ! do iele=1,n_ele
    !     if(my_id .eq. 1) write(*,"(9I5)"),e2n_pos(iele), e2n(e2n_pos(iele)+0), e2n(e2n_pos(iele)+1), e2n(e2n_pos(iele)+2), e2n(e2n_pos(iele)+3), e2n(e2n_pos(iele)+4), e2n(e2n_pos(iele)+5), e2n(e2n_pos(iele)+6), e2n(e2n_pos(iele)+7)
    ! end do

    ! 获取格点坐标
    allocate(coord(n_node*dim), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, coord'
    call get_coords(coord)

    ! 生成网格单元本地ID
    allocate(id(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, id'
    do iele=1,n_ele
        id(iele) = iele
    end do
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "id"//C_NULL_CHAR, id, n_dim)

    ! 计算网格单元体积
    allocate(vol(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, vol'
    call calc_eles_vol(n_blk, eblkt, eblkS, e2n, coord, vol)

    ! 注册体积场
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "vol"//C_NULL_CHAR, vol, n_dim)
    ! call get_scalar_field("cell"//C_NULL_CHAR, "vol"//C_NULL_CHAR, vol_new2, &
    !     & ndim_new, n_ele_new)

    call get_inn_face_blk_num(n_blk)
    write(*,*), "face blocks num: ", n_blk
    allocate(fblkt(n_blk), stat=err_mem)
    ! if(err_mem .ne. 0) stop 'Error, fails to allocate memory, eblk
    ! 获取网格面类型数目及起始位置'
    call get_inn_face_blk_type(fblkt)
    allocate(fblkS(n_blk), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, eblkS'
    call get_inn_face_blk_pos(fblkS)

    ! 计算网格面所含格点总数
    face_nodes = 0
    do iblk=1,n_blk
        face_nodes = face_nodes + nodes_num_for_ele(fblkt(iblk))*(fblkS(iblk+1)-fblkS(iblk))
    end do
    write(*,*), "nodes num in faces: ", face_nodes

    ! 获取网格面与格点拓扑关系
    allocate(if2n(face_nodes), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2n'
    call get_inn_face_2_node_blk(if2n)

    ! 获取内部网格面与网格单元拓扑关系
    allocate(if2e(n_face*2), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2e'
    call get_inn_face_2_ele_blk(if2e)
    ! do iface=1,n_face
    !     write(*,*),iface, if2e((iface-1)*2+1), if2e((iface-1)*2+2)
    ! end do
    ! SpMV test
    ! b = A*x
    allocate(x(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory x'

    allocate(b(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory b'

    allocate(A(n_face), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory a'

    x=1
    b=0
    A=1

    call add_scalar_field("cell"//C_NULL_CHAR, "b"//C_NULL_CHAR, b, n_dim)
    call add_scalar_field("cell"//C_NULL_CHAR, "x"//C_NULL_CHAR, x, n_dim)
    call add_scalar_field("face"//C_NULL_CHAR, "A"//C_NULL_CHAR, A, n_dim)

    b_tmp = get_scalar_field("cell"//C_NULL_CHAR, "b"//C_NULL_CHAR)
    call c_f_pointer(b_tmp%data, b_new, [n_ele+b_tmp%nbrSize])

    x_tmp = get_scalar_field("cell"//C_NULL_CHAR, "x"//C_NULL_CHAR)
    call c_f_pointer(x_tmp%data, x_new, [n_ele+b_tmp%nbrSize])

    A_tmp = get_scalar_field("face"//C_NULL_CHAR, "A"//C_NULL_CHAR)
    call c_f_pointer(A_tmp%data, A_new, [n_face])

    ! compute internal part
    call start_exchange_scalar_field("cell"//C_NULL_CHAR, "x"//C_NULL_CHAR)
    face_start = 1
    face_end = n_face_i
    call calc_spmv_gu(face_start, face_end, if2e, A_new, x_new, b_new, n_dim)
    ! compute processor boundary part
    call finish_exchange_scalar_field("cell"//C_NULL_CHAR, "x"//C_NULL_CHAR)
    face_start = n_face_i+1
    face_end = n_face
    call calc_spmv_gu(face_start, face_end, if2e, A_new, x_new, b_new, n_dim)


    ! 获取结果网格文件名
    nPara = 4
    call get_string_para(result_file, str_len, nPara, &
        & "domain1"//C_NULL_CHAR, &
        & "region"//C_NULL_CHAR, &
        & "0"//C_NULL_CHAR, &
        & "resPath"//C_NULL_CHAR)

    ! 输出网格到CGNS文件中
    call write_mesh(result_file)
    ! 输出体积场到CGNS文件中
    call write_scalar_field(result_file, "vol"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出b场到CGNS文件中
    call write_scalar_field(result_file, "b"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出pid场到CGNS文件中
    call write_scalar_field(result_file, "pid"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出ID场到CGNS文件中
    call write_scalar_field(result_file, "id"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出面积场到CGNS文件中，目前非结构网格只能输出格点和格心的求解值
    ! call write_scalar_field("b"//C_NULL_CHAR, "face"//C_NULL_CHAR)

    ! 获取边界网格面类型数目
    call get_bnd_face_blk_num(n_blk)
    write(*,*), "boundary face block num: ",n_blk
    ! 获取边界网格面类型
    if(allocated(fblkt)) deallocate(fblkt)
    allocate(fblkt(n_blk), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, fblkt'
    call get_bnd_face_blk_type(fblkt)
    ! 获取边界网格面起始位置
    if(allocated(fblkS)) deallocate(fblkS)
    allocate(fblkS(n_blk), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, eblkS'
    call get_bnd_face_blk_pos(fblkS)

    ! 计算边界网格面所含格点总数
    face_nodes = 0
    do iblk=1,n_blk
        face_nodes = face_nodes + nodes_num_for_ele(fblkt(iblk))*(fblkS(iblk+1)-fblkS(iblk))
    end do
    ! 获取边界网格面与格点拓扑关系
    allocate(bf2n(face_nodes), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, bf2n'
    call get_bnd_face_2_node_blk(bf2n)

    ! 获取边界网格面与网格单元拓扑关系
    allocate(bf2e(n_face_b), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, bf2e'
    call get_bnd_face_2_ele_blk(bf2e)
    ! 计算边界面上的spmv
    call calc_spmv_gu_bnd(1, n_face_b, bf2e, b_new)

    ! 再次输出b场到CGNS文件中
    call write_scalar_field(result_file, "b"//C_NULL_CHAR, "cell"//C_NULL_CHAR)

    ! 获取进程间极值
    local_val = my_id
    max_val = -1
    min_val = 200
    count = 1
    ! 最大值
    call extreme_labels_in_procs("MAX"//C_NULL_CHAR, local_val, max_val, count)
    ! write(*,*),my_id,max_val
    local_real = 1/real(my_id+1)
    min_real = 100.0
    ! 最小值
    call extreme_scalars_in_procs("MIN"//C_NULL_CHAR, local_real, min_real, count)
    ! write(*,*),my_id,min_real

    ! 获取边界网格面边界类型
    allocate(bnd_type(n_face_b), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, bnd_type'
    call get_bnd_type(bnd_type)
    ! do iface=1,n_face_b
    !     if(my_id .eq. 1) write(*,*),iface,bnd_type(iface)
    ! end do

    call clear()
    ! f_ptr => test_f
    ! ndim = f_ptr()

end program main

subroutine calc_eles_vol(n_blk, eblkt, eblkS, e2n, coord, vol)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN) :: n_blk
    integer(dpI), intent(IN) :: e2n(*), eblkt(*), eblkS(*)
    real(dpR), intent(IN) :: coord(*)
    real(dpR), intent(INOUT) :: vol(*)
    real(dpR), allocatable:: nodes_coord(:)
    integer(dpI):: iele,ie2n,inode,n_node,idim,idx,iblk,nodes_num_for_ele
    integer(dpI):: node_idx, node_id

    allocate(nodes_coord(max_n_node*dim), stat=err_mem)
    node_idx = 0;
    do iblk=1,n_blk
        n_node = nodes_num_for_ele(eblkt(iblk))
        ! write(*,*), eblkS(iblk), eblkS(iblk+1)-1
        do iele=eblkS(iblk), eblkS(iblk+1)-1
            do inode=1,n_node
                node_id  = e2n(node_idx+inode)
                do idim=1,dim
                    nodes_coord((inode-1)*dim+idim) = coord((node_id-1)*dim+idim)
                end do
            end do
            node_idx = node_idx + n_node
            ! write(*,*),iele
            call calc_vol(nodes_coord, eblkt(iblk), vol(iele))
            ! write(*,*), iblk, iele, vol(iele)
        end do
    end do

    return
end subroutine calc_eles_vol

subroutine calc_vol(nodes_coord, ele_type, vol)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI),intent(IN):: ele_type
    real(dpR),intent(IN):: nodes_coord(*)
    real(dpR),intent(INOUT):: vol
    if(ele_type .eq. 17 .or. ele_type .eq. 19) then
        call calc_HEXA_vol(nodes_coord, vol)
	else if(ele_type .eq. 10) then
        call calc_TETRA_vol(nodes_coord, vol)
    else if(ele_type .eq. 12) then
        call calc_PYRA_vol(nodes_coord, vol)
    else if(ele_type .eq. 5) then
        call calc_TRI_area(nodes_coord, vol)
    else if(ele_type .eq. 7) then
        call calc_QUAD_area(nodes_coord, vol)
    else
        write(*,*), "the element type is not supported"
    end if
end subroutine calc_vol

subroutine calc_faces_area(n_blk, fblkt, fblkS, f2n, coord, area)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN) :: n_blk
    integer(dpI), intent(IN) :: f2n(*), fblkt(*), fblkS(*)
    real(dpR), intent(IN) :: coord(*)
    real(dpR), intent(INOUT) :: area(*)
    real(dpR), allocatable:: nodes_coord(:)
    integer(dpI):: iele,ie2n,inode,n_node,idim,idx,iblk,nodes_num_for_ele
    integer(dpI):: node_idx, node_id

    allocate(nodes_coord(max_n_node*dim), stat=err_mem)
    node_idx = 0;
    do iblk=1,n_blk
        n_node = nodes_num_for_ele(fblkt(iblk))
        ! write(*,*), eblkS(iblk), eblkS(iblk+1)-1
        do iele=fblkS(iblk), fblkS(iblk+1)-1
            do inode=1,n_node
                node_id  = f2n(node_idx+inode)
                do idim=1,dim
                    nodes_coord((inode-1)*dim+idim) = coord((node_id-1)*dim+idim)
                end do
            end do
            node_idx = node_idx + n_node
            call calc_vol(nodes_coord, fblkt(iblk), area(iele))
            ! write(*,*), iblk, iele, area(iele)
        end do
    end do

    return
    
end subroutine calc_faces_area

subroutine calc_HEXA_vol(coord, vol)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: vol
    real(dpR):: v1(dim), v2(dim), v3(dim)

! write(*,*),coord(0*dim+1), coord(0*dim+2), coord(0*dim+3)
! write(*,*),coord(1*dim+1), coord(1*dim+2), coord(1*dim+3)
! write(*,*),coord(2*dim+1), coord(2*dim+2), coord(2*dim+3)
! write(*,*),coord(3*dim+1), coord(3*dim+2), coord(3*dim+3)
! write(*,*),coord(4*dim+1), coord(4*dim+2), coord(4*dim+3)
    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = coord(5)-coord(1)
    v3(1) = coord(4*dim+1)-coord(0*dim+1)
    v3(2) = coord(4*dim+2)-coord(0*dim+2)
    v3(3) = coord(4*dim+3)-coord(0*dim+3)

    ! vol = (v1xv2)*v3
    vol =     v3(1)*(v1(2)*v2(3)-v2(2)*v1(3))
    vol = vol+v3(2)*(v1(3)*v2(1)-v1(1)*v2(3))
    vol = vol+v3(3)*(v1(1)*v2(2)-v2(1)*v1(2))

    ! write(*,*), vol
    
end subroutine calc_HEXA_vol

subroutine calc_TETRA_vol(coord, vol)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: vol
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(3)-coord(1)
    v2(1) = coord(2*dim+1)-coord(0*dim+1)
    v2(2) = coord(2*dim+2)-coord(0*dim+2)
    v2(3) = coord(2*dim+3)-coord(0*dim+3)

    ! v3 = coord(4)-coord(1)
    v3(1) = coord(3*dim+1)-coord(0*dim+1)
    v3(2) = coord(3*dim+2)-coord(0*dim+2)
    v3(3) = coord(3*dim+3)-coord(0*dim+3)

    ! vol = (v1xv2)*v3
    vol =     v3(1)*(v1(2)*v2(3)-v2(2)*v1(3))
    vol = vol+v3(2)*(v1(3)*v2(1)-v1(1)*v2(3))
    vol = vol+v3(3)*(v1(1)*v2(2)-v2(1)*v1(2))
    vol = abs(vol)/6
    
end subroutine calc_TETRA_vol

subroutine calc_PYRA_vol(coord, vol)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: vol
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! write(*,*),"coord(1): ",coord(0*dim+1),coord(0*dim+2),coord(0*dim+3)
    ! write(*,*),"coord(2): ",coord(1*dim+1),coord(1*dim+2),coord(1*dim+3)
    ! write(*,*),"coord(3): ",coord(2*dim+1),coord(2*dim+2),coord(2*dim+3)
    ! write(*,*),"coord(4): ",coord(3*dim+1),coord(3*dim+2),coord(3*dim+3)
    ! write(*,*),"coord(5): ",coord(4*dim+1),coord(4*dim+2),coord(4*dim+3)
    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = coord(5)-coord(1)
    v3(1) = coord(4*dim+1)-coord(0*dim+1)
    v3(2) = coord(4*dim+2)-coord(0*dim+2)
    v3(3) = coord(4*dim+3)-coord(0*dim+3)

    ! vol = (v1xv2)*v3
    vol =     v3(1)*(v1(2)*v2(3)-v2(2)*v1(3))
    vol = vol+v3(2)*(v1(3)*v2(1)-v1(1)*v2(3))
    vol = vol+v3(3)*(v1(1)*v2(2)-v2(1)*v1(2))
    vol = abs(vol)/3
    ! write(*,*),vol
    
end subroutine calc_PYRA_vol

subroutine calc_QUAD_area(coord, area)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: area
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = v1xv2
    v3(1) = v1(2)*v2(3)-v2(2)*v1(3)
    v3(2) = v1(3)*v2(1)-v1(1)*v2(3)
    v3(3) = v1(1)*v2(2)-v2(1)*v1(2)

    area = sqrt(v3(1)*v3(1)+v3(2)*v3(2)+v3(3)*v3(3))

    ! write(*,*), area

end subroutine calc_QUAD_area

subroutine calc_TRI_area(coord, area)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: area
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = v1xv2
    v3(1) = v1(2)*v2(3)-v2(2)*v1(3)
    v3(2) = v1(3)*v2(1)-v1(1)*v2(3)
    v3(3) = v1(1)*v2(2)-v2(1)*v1(2)

    area = sqrt(v3(1)*v3(1)+v3(2)*v3(2)+v3(3)*v3(3))/2

end subroutine calc_TRI_area

subroutine calc_spmv(index_start, index_end, f2c, A, x, b)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN):: f2c(*)
    real(dpR), intent(IN):: A(*), x(*)
    real(dpR), intent(INOUT):: b(*)
    integer(dpI), intent(IN):: index_start, index_end
    integer:: iface, row, col

    do iface=index_start,index_end
        row    = f2c((iface-1)*2+1)
        col    = f2c((iface-1)*2+2)
        b(row) = b(row)+A(iface)*x(col)
        b(col) = b(col)+A(iface)*x(row)
    end do
    
end subroutine calc_spmv

subroutine calc_spmv_gu(index_start, index_end, f2c, A, x, b, ndim)
  use var_kind_def
  use var_global
  use iso_c_binding
  implicit none
  integer(dpI), intent(IN):: f2c(*)
  real(dpR), intent(IN):: A(*), x(*)
  real(dpR), intent(INOUT):: b(*)
  integer(dpI), intent(IN):: index_start, index_end, ndim
  integer:: iface, row, col, idim, tmp1, tmp2

  ! call MPI_Barrier(MPI_COMM_WORLD,ierr)

  do iface=index_start, index_end
      row    = f2c((iface-1)*2+1)
      col    = f2c((iface-1)*2+2)
      do idim = 0, ndim-1
        b(row*ndim+idim) = b(row*ndim+idim)+A(iface)*x(col*ndim+idim)
        ! this routine is not essential for processor boundary part computing
        b(col*ndim+idim) = b(col*ndim+idim)+A(iface)*x(row*ndim+idim)
      end do
  end do
end subroutine calc_spmv_gu

subroutine calc_spmv_gu_bnd(index_start, index_end, f2c, b)
  use var_kind_def
  use var_global
  use iso_c_binding
  implicit none
  integer(dpI), intent(IN):: f2c(*)
  real(dpR), intent(INOUT):: b(*)
  integer(dpI), intent(IN):: index_start, index_end
  integer:: iface, row, col

  ! call MPI_Barrier(MPI_COMM_WORLD,ierr)

  do iface=index_start, index_end
      row    = f2c(iface)
      b(row) = b(row)+1
      ! write(*,*), iface,row,b(row)
  end do
end subroutine calc_spmv_gu_bnd

function nodes_num_for_ele(ele_type)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI),intent(IN):: ele_type
    integer(dpI):: nodes_num_for_ele

    if(ele_type .eq. 10) then
        nodes_num_for_ele = 4
    else if(ele_type .eq. 12) then
        nodes_num_for_ele = 5
    else if(ele_type .eq. 5 ) then
        nodes_num_for_ele = 3
    else if(ele_type .eq. 7 ) then
        nodes_num_for_ele = 4
    else if(ele_type .eq. 17) then
        nodes_num_for_ele = 8
    else 
        stop "the element type is not supported"
    end if

    return
end function nodes_num_for_ele

function faces_num_for_ele(ele_type)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI),intent(IN):: ele_type
    integer(dpI):: faces_num_for_ele

    if(ele_type .eq. 10) then
        faces_num_for_ele = 4
    else if(ele_type .eq. 12) then
        faces_num_for_ele = 5
    else if(ele_type .eq. 17) then
        faces_num_for_ele = 6
    else 
        stop "the element type is not supported"
    end if

    return
end function faces_num_for_ele