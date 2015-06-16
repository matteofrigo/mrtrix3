#include "dwi/tractography/connectomics/point2mesh_mapper.h"


namespace MR
{

namespace DWI
{

namespace Tractography
{

namespace Connectomics
{


Point2MeshMapper::Point2MeshMapper( Mesh::SceneModeller* sceneModeller,
                                    float distanceLimit )
                 : _sceneModeller( sceneModeller ),
                   _distanceLimit( distanceLimit )
{

  // preparing index = 0 for failing to find an associated polygon
  uint32_t polygonIndex = 0;
  _polygonLut[ Point< int32_t >( 0, 0, 0 ) ] = polygonIndex;
  ++ polygonIndex;

  // preparing a polygon lut
  int32_t polygonCount = 0;
  int32_t sceneMeshCount = _sceneModeller->getSceneMeshCount();
  for ( int32_t m = 0; m < sceneMeshCount; m++ )
  {

    Mesh::SceneMesh* currentSceneMesh = _sceneModeller->getSceneMesh( m );
    Mesh::PolygonList polygons = currentSceneMesh->getMesh()->polygons;
    polygonCount = currentSceneMesh->getPolygonCount();
    for ( int32_t p = 0; p < polygonCount; p++ )
    {

      _polygonLut[ Point< int32_t >( polygons[ p ].indices[ 0 ],
                                     polygons[ p ].indices[ 1 ],
                                     polygons[ p ].indices[ 2 ] )
                  ] = polygonIndex;
      ++ polygonIndex;

    }

  }

}


Point2MeshMapper::~Point2MeshMapper()
{
}


void Point2MeshMapper::findNodePair( const Streamline< float >& tck,
                                     NodePair& nodePair )
{

  uint32_t node1 = getNodeIndex( tck.front() );
  uint32_t node2 = getNodeIndex( tck.back() );
  if ( node1 <= node2 )
  {

    nodePair.setNodePair( node1, node2 );

  }
  else
  {

    nodePair.setNodePair( node2, node1 );

  }

}


uint32_t Point2MeshMapper::getNodeCount() const
{

  return _polygonLut.size();

}


uint32_t Point2MeshMapper::getNodeIndex( const Point< float >& point ) const
{

  float distance = std::numeric_limits< float >::infinity();
  Mesh::SceneMesh* sceneMesh;
  Mesh::Polygon< 3 > polygon;
  if ( findNode( point, distance, sceneMesh, polygon ) )
  {

    return _polygonLut.find( Point< int32_t >( polygon.indices[ 0 ],
                                               polygon.indices[ 1 ],
                                               polygon.indices[ 2 ] ) )->second;

  }
  else
  {

    return 0;

  }

}


bool Point2MeshMapper::findNode( const Point< float >& point,
                                 float& distance,
                                 Mesh::SceneMesh*& sceneMesh,
                                 Mesh::Polygon< 3 >& polygon ) const
{

  // initialising empty scene mesh
  Mesh::SceneMesh* closestMesh = NULL;

  if ( !_sceneModeller->getSceneMeshes().empty() )
  {

    // getting the local voxel from the given point
    Point< int32_t > voxel;
    _sceneModeller->getBresenhamLineAlgorithm().getVoxelFromPoint( point,
                                                                   voxel );

    // getting the minimum resolution of the cache voxel
    Point< float > resolution = _sceneModeller->getResolution();
    float minimumResolution = std::min( std::min( resolution[ 0 ],
                                                  resolution[ 1 ] ),
                                                  resolution[ 2 ] );

    // increasing searching distance until reaching the limit (threshold)
    Point< int32_t > v;
    float newDistance;
    Mesh::Polygon< 3 > newPolygon;
    Point< float > newPoint;

    float searchingDistance = minimumResolution;
    int32_t stride = 1;
    do
    {

      for ( int32_t x = -stride; x <= stride; x++ )
      {

        v[ 0 ] = voxel[ 0 ] + x;
        for ( int32_t y = -stride; y <= stride; y++ )
        {

          v[ 1 ] = voxel[ 1 ] + y;
          for ( int32_t z = -stride; z <= stride; z++ )
          {

            v[ 2 ] = voxel[ 2 ] + z;
            if ( _sceneModeller->getIntegerBoundingBox().contains( v ) )
            {

              // collecting meshes crossing this voxel
              std::vector< Mesh::SceneMesh* > meshes =
                _sceneModeller->getSceneMeshCache().getSceneMeshes( v );

              // if some meshes are found in the current voxel
              if ( !meshes.empty() )
              {

                // looping over the meshes
                std::vector< Mesh::SceneMesh* >::const_iterator
                  m = meshes.begin(),
                  me = meshes.end();
                while ( m != me )
                {

                  // getting the distance to the closest polygon
                  ( *m )->getClosestPolygonAtVoxel( point,
                                                    v,
                                                    newDistance,
                                                    newPolygon,
                                                    newPoint );

                  if ( newDistance < distance && newDistance <= _distanceLimit )
                  {

                    // updating distance / mesh / polygon
                    distance = newDistance;
                    closestMesh = *m;
                    polygon = newPolygon;

                  }
                  ++ m;

                }

              }

            }

          }

        }

      }
      searchingDistance += minimumResolution;
      ++ stride;

    }
    while ( closestMesh == 0 && searchingDistance <= _distanceLimit );

  }

  // checking whether if the closest mesh/polygon exists
  if ( closestMesh != 0 )
  {

    return true;

  }
  else
  {

    distance = std::numeric_limits< float >::infinity(); 
    sceneMesh = closestMesh;
    polygon[ 0 ] = polygon[ 1 ] = polygon[ 2 ] = 0;
    return false;

  }

}


}

}

}

}

