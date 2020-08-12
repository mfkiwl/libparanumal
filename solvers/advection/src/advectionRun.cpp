/*

The MIT License (MIT)

Copyright (c) 2017 Tim Warburton, Noel Chalmers, Jesse Chan, Ali Karakus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "advection.hpp"

void advection_t::Run(){

  dfloat startTime, finalTime;
  settings.getSetting("START TIME", startTime);
  settings.getSetting("FINAL TIME", finalTime);

  initialConditionKernel(mesh.Nelements,
                         startTime,
                         mesh.o_x,
                         mesh.o_y,
                         mesh.o_z,
                         o_q);

  timeStepper->Run(o_q, startTime, finalTime);

  // output norm of final solution
  {
    //compute q.M*q
    MassMatrixKernel(mesh.Nelements, mesh.o_ggeo, mesh.o_MM, o_q, o_Mq);

    dlong Nentries = mesh.Nelements*mesh.Np;
    dfloat norm2 = sqrt(linAlg.innerProd(Nentries, o_q, o_Mq, comm));

    if(mesh.rank==0)
      printf("Solution norm = %17.15lg\n", norm2);
  }

}
