#!/usr/bin/env python3

#####################################################################################
#
#The MIT License (MIT)
#
#Copyright (c) 2017 Tim Warburton, Noel Chalmers, Jesse Chan, Ali Karakus
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.
#
#####################################################################################

from test import *

insData2D = insDir + "/data/insVortex2D.h"
insData3D = insDir + "/data/insBeltrami3D.h"

def insSettings(rcformat="2.0", data_file=insData2D,
               mesh="BOX", dim=2, element=4, nx=10, ny=10, nz=10, boundary_flag=2,
               degree=4, thread_model=device, platform_number=0, device_number=0,
               viscosity=0.05,
               advection_type="COLLOCATION",
               time_integrator="EXTBDF3", start_time=0.0, final_time=0.1,
               num_subcycles=4, subcycle_integrator="DOPRI5",
               velocity_discretization="CONTINUOUS",
               velocity_linear_solver="PCG",
               velocity_precon="JACOBI",
               velocity_multigrid_smoother="CHEBYSHEV",
               velocity_paralmond_cycle="VCYCLE",
               velocity_paralmond_smoother="CHEBYSHEV",
               pressure_discretization="CONTINUOUS",
               pressure_linear_solver="FPCG",
               pressure_precon="MULTIGRID",
               pressure_multigrid_smoother="CHEBYSHEV",
               pressure_paralmond_cycle="KCYCLE",
               pressure_paralmond_smoother="CHEBYSHEV"):
  return [setting_t("FORMAT", rcformat),
          setting_t("DATA FILE", data_file),
          setting_t("MESH FILE", mesh),
          setting_t("MESH DIMENSION", dim),
          setting_t("ELEMENT TYPE", element),
          setting_t("BOX NX", nx),
          setting_t("BOX NY", ny),
          setting_t("BOX NZ", nz),
          setting_t("BOX DIMX", 1),
          setting_t("BOX DIMY", 1),
          setting_t("BOX DIMZ", 1),
          setting_t("BOX BOUNDARY FLAG", boundary_flag),
          setting_t("POLYNOMIAL DEGREE", degree),
          setting_t("THREAD MODEL", thread_model),
          setting_t("PLATFORM NUMBER", platform_number),
          setting_t("DEVICE NUMBER", device_number),
          setting_t("VISCOSITY", viscosity),
          setting_t("ADVECTION TYPE", advection_type),
          setting_t("TIME INTEGRATOR", time_integrator),
          setting_t("NUMBER OF SUBCYCLES", num_subcycles),
          setting_t("SUBCYCLING TIME INTEGRATOR", subcycle_integrator),
          setting_t("START TIME", start_time),
          setting_t("FINAL TIME", final_time),
          setting_t("VELOCITY DISCRETIZATION", velocity_discretization),
          setting_t("VELOCITY LINEAR SOLVER", velocity_linear_solver),
          setting_t("VELOCITY PRECONDITIONER", velocity_precon),
          setting_t("VELOCITY MULTIGRID SMOOTHER", velocity_multigrid_smoother),
          setting_t("VELOCITY PARALMOND CYCLE", velocity_paralmond_cycle),
          setting_t("VELOCITY PARALMOND SMOOTHER", velocity_paralmond_smoother),
          setting_t("VELOCITY VERBOSE", "TRUE"),
          setting_t("PRESSURE DISCRETIZATION", pressure_discretization),
          setting_t("PRESSURE LINEAR SOLVER", pressure_linear_solver),
          setting_t("PRESSURE PRECONDITIONER", pressure_precon),
          setting_t("PRESSURE MULTIGRID SMOOTHER", pressure_multigrid_smoother),
          setting_t("PRESSURE PARALMOND CYCLE", pressure_paralmond_cycle),
          setting_t("PRESSURE PARALMOND SMOOTHER", pressure_paralmond_smoother),
          setting_t("PRESSURE VERBOSE", "TRUE"),
          setting_t("OUTPUT TO FILE", "FALSE")]

def main():
  failCount=0;

  #test non-subcycle
  failCount += test(name="testInsTri",
                    cmd=insBin,
                    settings=insSettings(element=3,data_file=insData2D,dim=2),
                    referenceNorm=0.820792600394893)

  failCount += test(name="testInsQuad",
                    cmd=insBin,
                    settings=insSettings(element=4,data_file=insData2D,dim=2),
                    referenceNorm=0.820483555285512)

  failCount += test(name="testInsTet",
                    cmd=insBin,
                    settings=insSettings(element=6,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2),
                    referenceNorm=1.20768114201421)

  failCount += test(name="testInsHex",
                    cmd=insBin,
                    settings=insSettings(element=12,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2),
                    referenceNorm=1.20751891719514)

  #test cubature
  failCount += test(name="testInsTri_cub",
                    cmd=insBin,
                    settings=insSettings(element=3,data_file=insData2D,dim=2,
                                         advection_type="CUBATURE"),
                    referenceNorm=0.820792629049566)

  failCount += test(name="testInsQuad_cub",
                    cmd=insBin,
                    settings=insSettings(element=4,data_file=insData2D,dim=2,
                                         advection_type="CUBATURE"),
                    referenceNorm=0.820483554297878)

  failCount += test(name="testInsTet_cub",
                    cmd=insBin,
                    settings=insSettings(element=6,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         advection_type="CUBATURE"),
                    referenceNorm=1.20772892218052)

  failCount += test(name="testInsHex_cub",
                    cmd=insBin,
                    settings=insSettings(element=12,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         advection_type="CUBATURE"),
                    referenceNorm=1.20756590561643)

  #test subcycle
  failCount += test(name="testInsTri_ss",
                    cmd=insBin,
                    settings=insSettings(element=3,data_file=insData2D,dim=2,
                                         time_integrator="SSBDF3"),
                    referenceNorm=0.820259718324697)

  failCount += test(name="testInsQuad_ss",
                    cmd=insBin,
                    settings=insSettings(element=4,data_file=insData2D,dim=2,
                                         time_integrator="SSBDF3"),
                    referenceNorm=0.819325040055746)

  failCount += test(name="testInsTet_ss",
                    cmd=insBin,
                    settings=insSettings(element=6,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         time_integrator="SSBDF3"),
                    referenceNorm=1.20564625692643)

  failCount += test(name="testInsHex_ss",
                    cmd=insBin,
                    settings=insSettings(element=12,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         time_integrator="SSBDF3"),
                    referenceNorm=1.20458245694617)

  #test cubature
  failCount += test(name="testInsTri_ss_cub",
                    cmd=insBin,
                    settings=insSettings(element=3,data_file=insData2D,dim=2,
                                         advection_type="CUBATURE",
                                         time_integrator="SSBDF3"),
                    referenceNorm=0.820259718324697)

  failCount += test(name="testInsQuad_ss_cub",
                    cmd=insBin,
                    settings=insSettings(element=4,data_file=insData2D,dim=2,
                                         advection_type="CUBATURE",
                                         time_integrator="SSBDF3"),
                    referenceNorm=0.819325040055746)

  failCount += test(name="testInsTet_ss_cub",
                    cmd=insBin,
                    settings=insSettings(element=6,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         advection_type="CUBATURE",
                                         time_integrator="SSBDF3"),
                    referenceNorm=1.20569732082373)

  failCount += test(name="testInsHex_ss_cub",
                    cmd=insBin,
                    settings=insSettings(element=12,data_file=insData3D,dim=3,
                                         nx=6, ny=6, nz=6, degree=2,
                                         advection_type="CUBATURE",
                                         time_integrator="SSBDF3"),
                    referenceNorm=1.20463427860078)

  #test wth MPI
  failCount += test(name="testInsTri_MPI", ranks=4,
                    cmd=insBin,
                    settings=insSettings(element=3,data_file=insData2D,dim=2),
                    referenceNorm=0.820785579220035)

  return failCount

if __name__ == "__main__":
  failCount=0;
  failCount+=main()
  sys.exit(failCount)