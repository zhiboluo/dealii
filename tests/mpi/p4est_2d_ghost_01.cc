// ---------------------------------------------------------------------
//
// Copyright (C) 2009 - 2020 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------



// check existence of ghost layer in 2d

#include <deal.II/base/tensor.h>
#include <deal.II/base/utilities.h>

#include <deal.II/distributed/tria.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>

#include "../tests.h"



template <int dim>
void
test()
{
  unsigned int myid     = Utilities::MPI::this_mpi_process(MPI_COMM_WORLD);
  unsigned int numprocs = Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD);

  if (true)
    {
      if (Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0)
        deallog << "hyper_cube" << std::endl;

      parallel::distributed::Triangulation<dim> tr(MPI_COMM_WORLD);

      GridGenerator::subdivided_hyper_cube(tr, 2);

      if (myid == 0)
        {
          std::vector<types::subdomain_id> cell_subd(tr.n_active_cells());

          GridTools::get_subdomain_association(tr, cell_subd);
          for (unsigned int i = 0; i < tr.n_active_cells(); ++i)
            deallog << cell_subd[i] << ' ';
          deallog << std::endl;
        }

      // check that all local
      // neighbors have the
      // correct level and a valid
      // subdomainid
      typename Triangulation<dim, dim>::active_cell_iterator cell;

      for (cell = tr.begin_active(); cell != tr.end(); ++cell)
        {
          if (cell->subdomain_id() != (unsigned int)myid)
            {
              AssertThrow(cell->is_ghost() || cell->is_artificial(),
                          ExcInternalError());
              continue;
            }

          for (const unsigned int n : GeometryInfo<dim>::face_indices())
            {
              if (cell->at_boundary(n))
                continue;
              AssertThrow(cell->neighbor(n).state() == IteratorState::valid,
                          ExcInternalError());

              AssertThrow(cell->neighbor(n)->level() == cell->level(),
                          ExcInternalError());

              AssertThrow(!cell->neighbor(n)->has_children(),
                          ExcInternalError());
              AssertThrow(cell->neighbor(n)->subdomain_id() < numprocs,
                          ExcInternalError());

              // all neighbors of
              // locally owned cells
              // must be ghosts but
              // can't be artificial
              AssertThrow(cell->neighbor(n)->is_ghost(), ExcInternalError());
              AssertThrow(!cell->neighbor(n)->is_artificial(),
                          ExcInternalError());
            }
        }

      const unsigned int checksum = tr.get_checksum();
      deallog << "Checksum: " << checksum << std::endl;
    }

  deallog << "OK" << std::endl;
}


int
main(int argc, char *argv[])
{
  Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);

  unsigned int myid = Utilities::MPI::this_mpi_process(MPI_COMM_WORLD);


  deallog.push(Utilities::int_to_string(myid));

  if (myid == 0)
    {
      initlog();

      deallog.push("2d");
      test<2>();
      deallog.pop();
    }
  else
    test<2>();
}
