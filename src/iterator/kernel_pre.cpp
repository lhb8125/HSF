#include "region.hpp"
#include "field.hpp"
#include "utilities.hpp"
#include "funPtr_host.hpp"
extern void prepareField(label32* inoutList, char* funcName, int nPara, ...);
extern void prepareConst(int nPara, ...);
void spMV_pre(Region& reg, Word A, Word x, Word b, long pi, S s, const int * arr){

	Field<scalar>& pre_fieldA = reg.getField<scalar>(A);
	Field<scalar>& pre_fieldx = reg.getField<scalar>(x);
	Field<scalar>& pre_fieldb = reg.getField<scalar>(b);

	Array<label32> inoutList;
	inoutList.push_back(1);
	inoutList.push_back(1);
	inoutList.push_back(3);

	prepareField(&inoutList[0], "spMV", 3, &pre_fieldA, &pre_fieldx, &pre_fieldb);
	prepareConst(6, &pi, sizeof(pi), &s, sizeof(s), arr, sizeof(*arr));
}

