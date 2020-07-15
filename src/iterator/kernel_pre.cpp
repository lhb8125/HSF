#include "region.hpp"
#include "field.hpp"
#include "utilities.hpp"
#include "funPtr_host.hpp"
#include "resource.hpp"
extern e2v_slaveFunPtr spMV_kernel;
void spMV_pre(Region& reg, Word A, Word x, Word b, long pi, S s, const int * arr)
{
	Field<scalar>& pre_fieldA = reg.getField<scalar>(A);
	Field<scalar>& pre_fieldx = reg.getField<scalar>(x);
	Field<scalar>& pre_fieldb = reg.getField<scalar>(b);

	Array<label32> inoutList;
	inoutList.push_back(1);
	inoutList.push_back(1);
	inoutList.push_back(3);

	prepareConst(6, &pi, sizeof(pi), &s, sizeof(s), arr, 10*sizeof(*arr));
	prepareField(&inoutList[0], spMV_kernel, 3, &pre_fieldA, &pre_fieldx, &pre_fieldb);
}

extern e2v_slaveFunPtr integration_kernel;
void integration_pre(Region& reg, Word flux, Word U)
{
	Field<scalar>& pre_fieldFlux = reg.getField<scalar>(flux);
	Field<scalar>& pre_fieldU = reg.getField<scalar>(U);

	Array<label32> inoutList;
	inoutList.push_back(1);
	inoutList.push_back(3);

	prepareConst(0);
	prepareField(&inoutList[0], integration_kernel, 2, &pre_fieldFlux, &pre_fieldU);
}


