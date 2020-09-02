#include "kernel_spe.hpp"
#include "resource.hpp"
#include "unat/iterator.h"
#include "kernel_slave.h"

void spMV_spe(Region& reg, Word A, Word x, Word b, label pi, StructS s, label32 * arr)
// void spMV_spe(Region& reg, Word A, Word x, Word b)
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

	constructParaSet(&paraData, 3, &pi, sizeof(pi), arr, 10*sizeof(*arr), &s, sizeof(s));
	// constructParaSet(&paraData, 3, &pi, sizeof(pi), &s, sizeof(s), arr, 10*sizeof(*arr));
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
	// prepareConst(0);
	// prepareField(&inoutList[0], integration_kernel, 2, &pre_fieldFlux, &pre_fieldU);
}

void calcLudsFcc_spe(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex, Word rface0, Word rface1, Word S)
{
    Field<scalar> &fMassFlux = reg.getField<scalar>(massFlux);
    Field<scalar> &fCellx    = reg.getField<scalar>(cellx);
    Field<scalar> &fFcc      = reg.getField<scalar>(fcc);
    Field<scalar> &fFacex    = reg.getField<scalar>(facex);
    Field<scalar> &fRface0   = reg.getField<scalar>(rface0);
    Field<scalar> &fRface1   = reg.getField<scalar>(rface1);
    Field<scalar> &fS        = reg.getField<scalar>(S);


	Array<Field<scalar>*> fieldList;
	fieldList.push_back(&fMassFlux);
	fieldList.push_back(&fCellx);
	fieldList.push_back(&fFcc);
	fieldList.push_back(&fFacex);
	fieldList.push_back(&fRface0);
	fieldList.push_back(&fRface1);
	fieldList.push_back(&fS);

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
