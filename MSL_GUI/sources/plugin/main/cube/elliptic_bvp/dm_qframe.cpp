/*  -*- c++ -*-

   +-----------------------------------------------------------------------+
   | MSL GUI - A Graphical User Interface for the Marburg Software Library |
   |                                                                       |
   | Copyright (C) 2018 Henning Zickermann                                 |
   | Contact: <zickermann@mathematik.uni-marburg.de>                       |
   +-----------------------------------------------------------------------+

     This file is part of MSL GUI.

     MSL GUI is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     MSL GUI is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with MSL GUI.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "dm_qframe.h"


namespace cube
{
namespace elliptic_bvp
{


using WaveletTL::TensorFrame;
using WaveletTL::TensorFrameEquation;


template <class IFRAME>
TensorFrame<IFRAME,2>* DM_QFrame::Problem<IFRAME>::createWaveletSystem(int jmax)
{
    MathTL::FixedArray1D<bool,4> bc;
    bc[0] = bc[1] = bc[2] = bc[3] = true;   // homogeneous Dirichlet boundary conditions

    TensorFrame<IFRAME,2>* frame = new TensorFrame<IFRAME,2>(bc);
    frame->set_jpmax(jmax, this->lastInput_.pmax);

    return frame;
}

template <class IFRAME>
TensorFrameEquation<IFRAME,2>*
DM_QFrame::Problem<IFRAME>::createDiscretizedEquation(const MathTL::EllipticBVP<2>& rawProblem,
                                                      const TensorFrame<IFRAME,2>& frame)
{
    return new TensorFrameEquation<IFRAME,2>(&rawProblem, frame);
}

}
}


// explicit template instantiation:
template class Instantiate<cube::elliptic_bvp::DM_QFrame>;