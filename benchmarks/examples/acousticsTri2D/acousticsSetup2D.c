#include "acoustics2D.h"

// NBN: toggle use of 2nd stream
#define USE_2_STREAMS
// #undef USE_2_STREAMS

void acousticsSetup2D(mesh2D *mesh){

  mesh->Nfields = 4;
  
  // compute samples of q at interpolation nodes
  mesh->q    = (dfloat*) calloc((mesh->totalHaloPairs+mesh->Nelements)*mesh->Np*mesh->Nfields,
				sizeof(dfloat));

  #if(TSTEP==LSERK)
    mesh->rhsq = (dfloat*) calloc(mesh->Nelements*mesh->Np*mesh->Nfields,
	 			sizeof(dfloat));
    mesh->resq = (dfloat*) calloc(mesh->Nelements*mesh->Np*mesh->Nfields,
	 			sizeof(dfloat));
  #elif (TSTEP==MRAB)
    mesh->rhsq  = (dfloat *) calloc(3*mesh->Nelements*mesh->Np*mesh->Nfields,sizeof(dfloat));
    mesh->rhsq2 = mesh->rhsq +   mesh->Nelements*mesh->Np*mesh->Nfields;
    mesh->rhsq3 = mesh->rhsq + 2*mesh->Nelements*mesh->Np*mesh->Nfields;
  #endif

  
  // fix this later (initial conditions)
  iint cnt = 0;
  for(iint e=0;e<mesh->Nelements;++e){
    for(iint n=0;n<mesh->Np;++n){
      dfloat t = 0;
      dfloat x = mesh->x[n + mesh->Np*e];
      dfloat y = mesh->y[n + mesh->Np*e];

#if 0
      acousticsCavitySolution2D(x, y, t,
				mesh->q+cnt, mesh->q+cnt+1, mesh->q+cnt+2);

      acousticsGaussianPulse2D(x, y, t,
			       mesh->q+cnt, mesh->q+cnt+1, mesh->q+cnt+2);
#endif
      mesh->q[cnt+2] = 1.;
      
      cnt += mesh->Nfields;
    }
  }

  // set penalty parameter
  mesh->Lambda2 = 0.5;
  
  // set time step
  dfloat hmin = 1e9;
  for(iint e=0;e<mesh->Nelements;++e){  

    for(iint f=0;f<mesh->Nfaces;++f){
      iint sid = mesh->Nsgeo*(mesh->Nfaces*e + f);
      dfloat sJ   = mesh->sgeo[sid + SJID];
      dfloat invJ = mesh->sgeo[sid + IJID];

      // sJ = L/2, J = A/2,   sJ/J = L/A = L/(0.5*h*L) = 2/h
      // h = 0.5/(sJ/J)
      
      dfloat hest = .5/(sJ*invJ);

      hmin = mymin(hmin, hest);
    }
  }
  dfloat cfl = .1; // depends on the stability region size

  dfloat dt = cfl*hmin/((mesh->N+1.)*(mesh->N+1.)*mesh->Lambda2);

  // MPI_Allreduce to get global minimum dt
  MPI_Allreduce(&dt, &(mesh->dt), 1, MPI_DFLOAT, MPI_MIN, MPI_COMM_WORLD);

  //
  mesh->finalTime = 4;
  mesh->NtimeSteps = mesh->finalTime/mesh->dt;
  mesh->dt = mesh->finalTime/mesh->NtimeSteps;

  // errorStep
  mesh->errorStep = 100;

  printf("dt = %g\n", mesh->dt);

  // OCCA build stuff
  char deviceConfig[BUFSIZ];
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // use rank to choose DEVICE
  sprintf(deviceConfig, "mode = CUDA, deviceID = %d", rank);
  //  sprintf(deviceConfig, "mode = OpenCL, deviceID = 0, platformID = 1");
  //  sprintf(deviceConfig, "mode = OpenMP, deviceID = %d", 1);

  occa::kernelInfo kernelInfo;

  // generic occa device set up
  meshOccaSetup2D(mesh, deviceConfig, kernelInfo);

  // specialization
  int maxNodes = mymax(mesh->Np, (mesh->Nfp*mesh->Nfaces));
  kernelInfo.addDefine("p_maxNodes", maxNodes);

  int NblockV = 512/mesh->Np; // works for CUDA
  kernelInfo.addDefine("p_NblockV", NblockV);

  int NblockS = 512/maxNodes; // works for CUDA
  kernelInfo.addDefine("p_NblockS", NblockS);
  
  kernelInfo.addDefine("p_Lambda2", 0.5f);


  
  mesh->volumeKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsVolume2D.okl",
				       "acousticsVolume2D_o0",
				       kernelInfo);

  mesh->surfaceKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsSurface2D.okl",
				       "acousticsSurface2D_s0",
				       kernelInfo);

  mesh->partialSurfaceKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsPartialSurface2D.okl",
				       "acousticsPartialSurface2D_s0",
				       kernelInfo);

  mesh->updateKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsUpdate2D.okl",
				       "acousticsUpdate2D",
				       kernelInfo);
  
  mesh->haloExtractKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/meshHaloExtract2D.okl",
				       "meshHaloExtract2D",
				       kernelInfo);

  mesh->pmlKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsPml2D.okl",
				       "acousticsPml2D",
				       kernelInfo);

  mesh->pmlUpdateKernel =
    mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsPmlUpdate2D.okl",
				       "acousticsPmlUpdate2D",
				       kernelInfo);

  
  int Ntests = 10;

#define maxNkernels 100

  int NvolumeKernels = 7;
  occa::kernel *acousticsVolumeKernels = new occa::kernel[maxNkernels];
  char kernelNames[maxNkernels][BUFSIZ];
  double bestElapsed = 1e9;
  
  for(int ker=0;ker<NvolumeKernels;++ker){
    sprintf(kernelNames[ker], "acousticsVolume2D_o%d", ker);
    
    acousticsVolumeKernels[ker] =
      mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsVolume2D.okl", kernelNames[ker], kernelInfo);
    
    mesh->device.finish();
    occa::tic(kernelNames[ker]);
    for(int test=0;test<Ntests;++test)
      acousticsVolumeKernels[ker](mesh->Nelements,
				      mesh->o_vgeo,
				      mesh->o_DrT,
				      mesh->o_DsT,
				      mesh->o_q,
				      mesh->o_rhsq);
    mesh->device.finish();
    double elapsed = occa::toc(kernelNames[ker]);
    if(elapsed<bestElapsed){
      mesh->volumeKernel = acousticsVolumeKernels[ker];
      printf("promoting kernel: %d (time %g)\n", ker, elapsed);
      bestElapsed = elapsed;
    }
    else{
      printf("not promoting kernel: %d (time %g)\n", ker, elapsed);
    }
  }

  int NsurfaceKernels = 6;
  char surfaceKernelNames[maxNkernels][BUFSIZ];
  occa::kernel *acousticsSurfaceKernels = new occa::kernel[maxNkernels];
  bestElapsed = 1e9;
  
  for(int ker=0;ker<NsurfaceKernels;++ker){
    sprintf(surfaceKernelNames[ker], "acousticsSurface2D_s%d", ker);
    
    acousticsSurfaceKernels[ker] =
      mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsSurface2D.okl",
					 surfaceKernelNames[ker], kernelInfo);
    
    mesh->device.finish();
    occa::tic(surfaceKernelNames[ker]);
    for(int test=0;test<Ntests;++test){
      dfloat t  = 0;
      acousticsSurfaceKernels[ker](mesh->Nelements,
				       mesh->o_sgeo,
				       mesh->o_LIFTT,
				       mesh->o_vmapM,
				       mesh->o_vmapP,
				       mesh->o_EToB,
				       t,
				       mesh->o_x,
				       mesh->o_y,
				       mesh->o_q,
				       mesh->o_rhsq);
    }
    mesh->device.finish();
    double elapsed = occa::toc(surfaceKernelNames[ker]);
    if(elapsed<bestElapsed){
      mesh->surfaceKernel = acousticsSurfaceKernels[ker];
      printf("promoting kernel: %d (time %g)\n", ker, elapsed);
      bestElapsed = elapsed;
    }
    else{
      printf("not promoting kernel: %d (time %g)\n", ker, elapsed);
    }
  }

  int NpartialSurfaceKernels = 6;
  char partialSurfaceKernelNames[maxNkernels][BUFSIZ];
  occa::kernel *acousticsPartialSurfaceKernels = new occa::kernel[maxNkernels];
  bestElapsed = 1e9;
  
  for(int ker=0;ker<NpartialSurfaceKernels;++ker){
    sprintf(partialSurfaceKernelNames[ker], "acousticsPartialSurface2D_s%d", ker);
    
    acousticsPartialSurfaceKernels[ker] =
      mesh->device.buildKernelFromSource(DHOLMES "/okl/acousticsPartialSurface2D.okl",
					 partialSurfaceKernelNames[ker], kernelInfo);
    
    mesh->device.finish();
    occa::tic(partialSurfaceKernelNames[ker]);
    for(int test=0;test<Ntests;++test){
      dfloat t  = 0;
      acousticsPartialSurfaceKernels[ker](mesh->NinternalElements,
					      mesh->o_internalElementIds,
					      mesh->o_sgeo,
					      mesh->o_LIFTT,
					      mesh->o_vmapM,
					      mesh->o_vmapP,
					      mesh->o_EToB,
					      t,
					      mesh->o_x,
					      mesh->o_y,
					      mesh->o_q,
					      mesh->o_rhsq);
    }
    mesh->device.finish();
    double elapsed = occa::toc(partialSurfaceKernelNames[ker]);
    if(elapsed<bestElapsed){
      mesh->partialSurfaceKernel = acousticsPartialSurfaceKernels[ker];
      printf("promoting kernel: %d (time %g)\n", ker, elapsed);
      bestElapsed = elapsed;
    }
    else{
      printf("not promoting kernel: %d (time %g)\n", ker, elapsed);
    }
  }

  // set up perfectly matched layer
  dfloat xmin = -.8, xmax = .8, ymin = -.8, ymax = .8;
  dfloat xsigma = 40, ysigma = 40;
  acousticsPmlSetup2D(mesh, xmin, xmax, ymin, ymax, xsigma, ysigma);
 

}
