/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
* @File: Field.hpp
* @Author: Hanfeng GU
* @Email:  hanfenggu@gmail.com
* @Date:   2019-09-18 16:03:45
* @Last Modified by:   Hanfeng GU
* @Last Modified time: 2019-11-12 10:27:07
*/

/*
* @brief:
*/

#ifndef Field_hpp
#define Field_hpp

#include "Patch.hpp"

namespace HSF
{

/**
 * @brief      This class describes a field.
 * @tparam     T     scalar, vector, tensor
 */
template<typename T>
class Field
{
private:
	label ndim_;
	label localSize_;
	T*    localData_;
	Table<Word, T*>* sendBufferPtr_;
	Table<Word, T*>* recvBufferPtr_;
	MPI_Request* sendRecvRequests_;
	Word  setType_;
	Table<Word, Patch*>* patchTabPtr_;

public:
	/**
	 * @brief      Constructs a new instance.
	 */
	Field(){}

	Field(Word setType, label ndim, label n, T* dataPtr)
	:
		setType_(setType),
		ndim_(ndim),
		localSize_(n),
		localData_(dataPtr)
	{}

	/**
	 * @brief      Destroys the object.
	 */
	~Field();

	/**
	 * @brief      Gets the size.
	 * @return     The size.
	 */
	inline label  getSize(){return localSize_;}

	/**
	 * @brief      Gets the data.
	 * @return     The local data.
	 */
	inline T*     getLocalData(){return localData_;}

	/**
	 * @brief      Gets the neighbor processor data.
	 * @return     The neighbor processor data.
	 */
	inline Table<Word, T*>* getNbrData(){return recvBufferPtr_;}

	/**
	 * @brief      Gets the type.
	 * @return     The type.
	 */
	inline Word   getType(){return setType_;}

	/**
	 * @brief      Initializes the send buffer and start iSend and iRecv.
	 */
	void initSend();

	/**
	 * @brief      check if we have receive the data from neighbor processors
	 * @return     if finished, return 1, else return 0
	 */
	label checkSendStatus();

	/**
	 * @brief      free the memory of communication
	 */
	void freeSendRecvBuffer();
};

#define scalarField Field<scalar>
#define labelField  Field<label>

} //- end namespace HSF
#endif //- end Field_hpp
