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


#ifndef INTERVAL_POISSON_BVP_PROBLEMTYPE_H
#define INTERVAL_POISSON_BVP_PROBLEMTYPE_H


#include "main/generic_problemtype_module.h"
#include "raw/sturm_bvp_containers.h"
#include "raw/elliptic_bvp_containers.h"

#include "dm_basis.h"
#include "dm_aggframe.h"
#include "dm_qframe.h"


namespace interval
{
namespace poisson_bvp
{

class ProblemType : public GenericProblemTypeModule< RawProblemModule<Poisson1DContainer, PoissonBvpContainer<1> >,
                                                     DM_Basis, DM_AggFrame, DM_QFrame >
{
    void setupNamesAndDescription() override;
    void setupExamples() override;
};

}
}

#endif // INTERVAL_POISSON_BVP_PROBLEMTYPE_H
