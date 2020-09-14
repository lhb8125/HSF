#include "kernel_spe.hpp"
#include "unat/iterator.hpp"
#include "kernel_slave.h"
#include "resource.h"
void spMV_spe(Region& reg, Word A, Word x, Word b, long pi, StructS s, const int * arr)
{
	Field<scalar>& pre_fieldA = reg.getField<scalar>(A);
	Field<scalar>& pre_fieldx = reg.getField<scalar>(x);
	Field<scalar>& pre_fieldb = reg.getField<scalar>(b);

	Array<Field<scalar>*> fieldList;
	fieldList.push_back(&pre_fieldA);
	fieldList.push_back(&pre_fieldx);
	fieldList.push_back(&pre_fieldb);

	Array<label32> inoutList;
	inoutList.push_back(0);
	inoutList.push_back(0);
	inoutList.push_back(1);

	constructParaSet(&paraData, 3, &pi, sizeof(pi), &s, sizeof(s), arr, 10*sizeof(*arr));
	prepareField(reg, inoutList, slave_spMV_skeleton, fieldList);
}

void integration_spe(Region& reg, Word flux, Word U)
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
}

void calcLudsFcc_spe(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex, Word rface0, Word rface1, Word S)
{
	Field<scalar>& pre_fMassFlux = reg.getField<scalar>(massFlux);
	Field<scalar>& pre_fCellx = reg.getField<scalar>(cellx);
	Field<scalar>& pre_fFcc = reg.getField<scalar>(fcc);
	Field<scalar>& pre_fFacex = reg.getField<scalar>(facex);
	Field<scalar>& pre_fRface0 = reg.getField<scalar>(rface0);
	Field<scalar>& pre_fRface1 = reg.getField<scalar>(rface1);
	Field<scalar>& pre_fS = reg.getField<scalar>(S);

	Array<Field<scalar>*> fieldList;
	fieldList.push_back(&pre_fMassFlux);
	fieldList.push_back(&pre_fCellx);
	fieldList.push_back(&pre_fFcc);
	fieldList.push_back(&pre_fFacex);
	fieldList.push_back(&pre_fRface0);
	fieldList.push_back(&pre_fRface1);
	fieldList.push_back(&pre_fS);

	Array<label32> inoutList;
	inoutList.push_back(0);
	inoutList.push_back(0);
	inoutList.push_back(1);
	inoutList.push_back(0);
	inoutList.push_back(1);
	inoutList.push_back(1);
	inoutList.push_back(1);

	constructParaSet(&paraData, 0);
	prepareField(reg, inoutList, slave_calcLudsFcc_skeleton, fieldList);
}


