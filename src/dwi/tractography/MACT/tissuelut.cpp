/*
 * Copyright (c) 2008-2016 the MRtrix3 contributors
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/
 * 
 * MRtrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * For more details, see www.mrtrix.org
 * 
 */


#include "dwi/tractography/MACT/tissuelut.h"
#include "dwi/tractography/MACT/scenemodeller.h"


namespace MR
{

namespace DWI
{

namespace Tractography
{

namespace MACT
{


TissueLut::TissueLut( const std::shared_ptr< SceneModeller >& sceneModeller )
          : _sceneModeller( sceneModeller )
{
}


TissueLut::~TissueLut()
{
}


std::set< Tissue_ptr >
TissueLut::getTissues( const Eigen::Vector3i& voxel ) const
{
  if ( _lut.count( voxel ) == 0 )
  {
    return std::set< Tissue_ptr >();
  }
  else
  {
    return _lut.find( voxel )->second;
  }
}


std::set< Tissue_ptr >
TissueLut::getTissues( const Eigen::Vector3d& point ) const
{
  Eigen::Vector3i voxel;
  _sceneModeller->lutVoxel( point, voxel );
  return getTissues( voxel );
}


void TissueLut::update( Tissue_ptr tissue )
{
  std::set< Eigen::Vector3i, Vector3iCompare > voxels;
  for ( Polygon_i p = 0; p < tissue->polygonCount(); p++ )
  {
    auto mesh = tissue->mesh();
    auto triangle = tissue->mesh().tri( p );
    auto v1 = mesh.vert( triangle[ 0 ] );
    auto v2 = mesh.vert( triangle[ 1 ] );
    auto v3 = mesh.vert( triangle[ 2 ] );

    // collecting associated voxels using Bresenham line algorithm
    _sceneModeller->bresenhamLine().discTriangleVoxels(
                        v1, v2, v3, tissue->radiusOfInfluence(), voxels, true );
    auto v = voxels.begin(), ve = voxels.end();
    while ( v != ve )
    {
      if ( _lut.count( *v ) == 0 )
      {
        // LUT does not have this voxel
        // --> initialise a new set of tissues and add to this voxel
        std::set< Tissue_ptr > newTissues{ tissue };
        _lut[ *v ] = newTissues;
      }
      else
      {
        // this voxel already contains some tissues
        // --> check if the input tissue is already inside the voxel
        bool doesNotExist = true;
        auto t = _lut[ *v ].begin(), te = _lut[ *v ].end();
        while ( t != te )
        {
          if ( ( *t ) == tissue )
          {
            doesNotExist = false;
          }
          ++ t;
        }
        if ( doesNotExist )
        {
          _lut[ *v ].insert( tissue );
        }
      }
      ++ v;
    }
  }
}


}

}

}

}
