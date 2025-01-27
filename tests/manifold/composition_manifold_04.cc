// ---------------------------------------------------------------------
//
// Copyright (C) 2016 - 2018 by the deal.II authors
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


// Stress periodicity in CompositionManifold. Compose PolarManifold with
// the identity, and make sure periodicity is respected.

#include "../tests.h"


// all include files you need here
#include <deal.II/grid/composition_manifold.h>
#include <deal.II/grid/manifold_lib.h>


int
main()
{
  initlog();
  std::ostream &out = deallog.get_file_stream();

  const int dim = 2, spacedim = 2;

  Point<spacedim> center;

  PolarManifold<dim, spacedim>              S(center);
  FunctionManifold<dim, spacedim, spacedim> F("x;y", "x;y");

  CompositionManifold<dim> manifold(S, F);

  // Chart points.
  Point<spacedim> cp[2];

  // Radius
  cp[0][0] = 1.0;
  cp[1][0] = 1.0;

  // Force roundoff errors on one side only
  double eps = 1e-10;

  // Last point
  cp[0][1] = -numbers::PI / 4;
  cp[1][1] = numbers::PI / 4 - eps;

  // Spacedim points
  std::vector<Point<spacedim>> sp(2);

  // Weights
  std::vector<double> w(2);

  sp[0] = manifold.push_forward(cp[0]);
  sp[1] = manifold.push_forward(cp[1]);

  unsigned int n_intermediates = 32;

  out << "set terminal aqua " << 0 << std::endl
      << "set size ratio -1" << std::endl
      << "plot '-' with vectors " << std::endl;

  for (unsigned int v = 0; v < sp.size(); ++v)
    out << center << ' ' << sp[v] << std::endl;


  for (unsigned int i = 0; i < n_intermediates + 1; ++i)
    {
      w[0] = 1.0 - (double)i / ((double)n_intermediates);
      w[1] = 1.0 - w[0];

      Point<spacedim> ip =
        manifold.get_new_point(make_array_view(sp), make_array_view(w));
      Tensor<1, spacedim> t1 = manifold.get_tangent_vector(ip, sp[0]);
      Tensor<1, spacedim> t2 = manifold.get_tangent_vector(ip, sp[1]);

      out << ip << ' ' << t2 << std::endl;
    }

  out << 'e' << std::endl;

  out << "set terminal aqua " << 1 << std::endl
      << "set size ratio -1" << std::endl
      << "plot '-' w lp " << std::endl;

  for (unsigned int i = 0; i < n_intermediates + 1; ++i)
    {
      w[0] = 1.0 - (double)i / ((double)n_intermediates);
      w[1] = 1.0 - w[0];

      Point<spacedim> ip = manifold.pull_back(
        manifold.get_new_point(make_array_view(sp), make_array_view(w)));

      ip[0] = w[1];

      out << ip << std::endl;
    }
  out << 'e' << std::endl;

  return 0;
}
