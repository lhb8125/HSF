#include "region.hpp"
#include "field.hpp"
#include "utilities.h"
// #include "funPtr_host.hpp"
#include "resource.hpp"
#include "unat/iterator.h"
#include "kernel_slave.h"
extern slaveFunPtr slave_spMV_kernel;

using namespace HSF;

void spMV_pre(Region& reg, Word A, Word x, Word b, long pi, S s, const int * arr)
{
	Field<scalar>& pre_fieldA = reg.getField<scalar>(A);
	Field<scalar>& pre_fieldx = reg.getField<scalar>(x);
	Field<scalar>& pre_fieldb = reg.getField<scalar>(b);

	Array<Field<scalar>*> fieldList(3);
	fieldList.push_back(&pre_fieldA);
	fieldList.push_back(&pre_fieldx);
	fieldList.push_back(&pre_fieldb);

	Array<label32> inoutList;
	inoutList.push_back(1);
	inoutList.push_back(1);
	inoutList.push_back(3);

	// constructParaSet(&paraData, 3, &pi, sizeof(pi), &s, sizeof(s), arr, 10*sizeof(*arr));
	// prepareField(reg, inoutList, spMV_kernel, fieldList);
}

void integration_pre(Region& reg, Word flux, Word U)
{
	Field<scalar>& pre_fieldFlux = reg.getField<scalar>(flux);
	Field<scalar>& pre_fieldU = reg.getField<scalar>(U);


	Array<Field<scalar>*> fieldList;
	fieldList.push_back(&pre_fieldFlux);
	fieldList.push_back(&pre_fieldU);

	Array<label32> inoutList;
	inoutList.push_back(0);
	inoutList.push_back(1);

	constructParaSet(&paraData, 0);
	prepareField(reg, inoutList, slave_integration_skeleton, fieldList);
	// prepareConst(0);
	// prepareField(&inoutList[0], integration_kernel, 2, &pre_fieldFlux, &pre_fieldU);
}

