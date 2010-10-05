/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
//
//  This file is part of the Computational Morphometry Toolkit.
//
//  http://www.nitrc.org/projects/cmtk/
//
//  The Computational Morphometry Toolkit is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  The Computational Morphometry Toolkit is distributed in the hope that it
//  will be useful, but WITHOUT ANY WARRANTY; without even the implied
//  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with the Computational Morphometry Toolkit.  If not, see
//  <http://www.gnu.org/licenses/>.
//
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/
#include <string.h>
#include <iostream>

#include <Base/cmtkTypes.h>
#include <Base/cmtkSplineWarpXform.h>

int
testSplineWarpXform()
{
  return 0;
}

int
testSplineWarpXformInverse()
{
  // parameters for the current WarpSingleLevel baseline transformation
  const cmtk::Types::Coordinate domain[3] = { 357.1875, 357.1875, 125 };
  const int dims[3] = { 7, 7, 4 };

  const size_t nparameters = 3 * 7 * 7 * 4;

  cmtk::Types::Coordinate parameters[nparameters] = 
    { -88.706249, -84.998951480000002, -133.41214640000001, 0.58282801900000003, -85.552844629999996, -132.6599301,
      89.312814939999996, -86.301460410000004, -132.5876548, 177.77193539999999, -85.646700699999997, -133.84530789999999,
      266.44951250000003, -79.496837150000005, -133.9639134, 355.83432440000001, -82.938156820000003, -133.6333545,
      444.48369489999999, -85.145062800000005, -134.2765378, -87.318911839999998, 3.4110965480000002, -132.63593660000001,
      2.617442912, -0.38841385519999999, -127.8795088, 90.052308600000003, -4.7583292430000004, -131.88906729999999,
      179.2010363, 0.1205040694, -133.54224679999999, 264.80709460000003, 12.066754080000001, -130.28005279999999,
      357.41610070000002, 7.3845064359999997, -127.76716690000001, 445.47999440000001, 3.1919038909999999, -132.9684795,
      -83.704470520000001, 92.192599630000004, -132.28900619999999, 4.9322056060000001, 90.245003139999994,
      -128.26304400000001, 87.099999409999995, 74.254342940000001, -135.77073709999999, 170.00687600000001,
      88.896090749999999, -155.3283208, 273.92401669999998, 93.242624550000002, -140.88084140000001, 361.17963400000002,
      89.683891509999995, -130.97434670000001, 445.75771220000001, 91.876312139999996, -133.26873420000001,
      -81.061337280000004, 181.6535792, -133.0931166, 1.0531317360000001, 188.02919120000001, -137.56335189999999,
      89.184044760000006, 178.8696482, -159.36757170000001, 174.015658, 183.60500909999999, -151.0189499, 267.2078841,
      179.12156770000001, -144.6624151, 359.74886040000001, 176.8779567, -137.3049566, 444.22113439999998,
      180.80086940000001, -133.6307272, -83.727375629999997, 267.29410710000002, -133.48054730000001, 1.7494822249999999,
      266.0169631, -132.43946500000001, 99.806269929999999, 267.21390309999998, -150.58490649999999, 177.13017690000001,
      262.75285589999999, -147.6204467, 260.4250389, 252.11927119999999, -145.32091500000001, 359.6639907, 257.1055576,
      -136.3139481, 445.63570859999999, 268.85830870000001, -133.20231140000001, -87.196138289999993, 356.09219039999999,
      -131.7662282, 5.5288343080000004, 348.41581830000001, -127.61307909999999, 93.661780930000006, 350.04663319999997,
      -134.64285319999999, 179.84697249999999, 356.04154419999998, -138.72237849999999, 263.79976570000002,
      351.84695269999997, -134.74277140000001, 354.08110850000003, 354.72597280000002, -127.8249725, 446.35394769999999,
      356.2593943, -132.68096320000001, -88.868390099999999, 445.57343600000002, -132.74548110000001, 0.13575482220000001,
      444.76912679999998, -131.80384480000001, 88.728966940000007, 444.62619749999999, -132.15681269999999,
      177.92944069999999, 446.30323390000001, -133.03929260000001, 266.35346240000001, 444.6431498, -132.9607939,
      355.45975850000002, 445.66228130000002, -133.26875509999999, 444.53735660000001, 445.56110649999999,
      -133.65817060000001, -87.595741469999993, -85.615120719999993, -5.958190739, 0.44238839149999998, -98.505404619999993,
      -2.2983611289999999, 91.516483010000002, -100.0147607, -5.1136814890000002, 181.5319556, -104.2308865,
      -5.8053919389999997, 268.33913130000002, -88.234155250000001, -6.6261121190000001, 357.53476169999999,
      -84.906273350000006, -3.6736957480000001, 445.84451159999998, -85.767281420000003, -6.19765224, -84.004465359999998,
      1.1194690920000001, -2.3237842579999999, 7.0343907919999999, 2.887315584, 9.3485539650000007, 85.201014270000002,
      3.2166232570000002, -2.638737415, 185.81284930000001, 3.2168928389999998, -5.0315402049999998, 263.85491830000001,
      -5.885358095, -1.5863716800000001, 355.65862499999997, -0.22509619959999999, 9.7554431780000002, 446.56552870000002,
      0.91509386459999997, -1.77461844, -76.155025309999999, 91.465387050000004, -4.5014921750000001, -0.025622953949999999,
      91.082948139999999, 36.132634940000003, 88.895439049999993, 91.715001909999998, 5.5567657759999998,
      175.59212350000001, 91.715271490000006, -19.827600650000001, 267.31410360000001, 91.71554107, -18.57972865,
      355.65862499999997, 92.067915659999997, 15.83988965, 447.8203795, 94.976408219999996, -1.8117929740000001,
      -89.183814949999999, 182.31435640000001, -0.18152812469999999, -0.025622953949999999, 180.213111, 29.576301829999998,
      88.895439049999993, 180.21338059999999, -50.387582109999997, 177.81650099999999, 187.63108, -6.3127768949999998,
      266.73756300000002, 188.21391969999999, -39.724707619999997, 355.65862499999997, 194.76894530000001,
      12.719229840000001, 448.49040489999999, 180.30507940000001, 6.4192943600000003, -86.124877290000001,
      266.36517220000002, 0.81955630999999995, -0.025622953949999999, 268.71148959999999, 6.6378779159999999,
      88.895439049999993, 268.71175920000002, -8.5170915610000009, 177.81650099999999, 268.71202879999998,
      -40.255468520000001, 266.73756300000002, 268.71229840000001, -11.29851287, 355.65862499999997, 268.71256799999998,
      12.631458200000001, 455.28980799999999, 262.68077749999998, 3.5113257330000001, -83.442058020000005,
      352.86904729999998, -1.6556360590000001, -0.025622953949999999, 357.20986829999998, 18.933949649999999,
      92.201104720000004, 357.21013790000001, -5.9082610569999998, 185.02496339999999, 357.21040749999997,
      -38.860956639999998, 265.44028609999998, 357.21067699999998, -14.704373049999999, 355.65862499999997, 357.2109466,
      9.0930626809999993, 444.68610519999999, 356.86132309999999, -1.78315417, -87.485908379999998, 444.2545389,
      -4.8936582450000001, 3.2569376710000002, 444.43092489999998, 0.096468254239999995, 91.317244149999993,
      442.34335479999999, -6.8668513320000004, 174.63876999999999, 450.46435989999998, -5.3950795060000001,
      265.36603500000001, 445.70905570000002, -3.9132990560000001, 354.38166330000001, 445.87286769999997,
      -4.1942974719999997, 445.27059109999999, 444.60602310000002, -6.1777497380000002, -87.057384740000003,
11      -86.151767390000003, 121.0374534, 1.7057175179999999, -96.19556394, 122.554754, 89.579146320000007, -104.5236975,
      118.37620750000001, 179.89012020000001, -104.9898585, 117.0390142, 267.55622369999998, -91.202323030000002,
      116.7009912, 355.50636229999998, -91.482698450000001, 121.1113952, 445.60660159999998, -86.026599090000005,
      120.60045580000001, -84.171902759999995, -0.01482944089, 121.7216818, 7.2181945880000002, 3.0001385580000002,
      107.25279829999999, 80.852307519999997, 3.0004081390000001, 107.9820965, 176.8311132, 3.0006777210000002, 120.5239799,
      263.74011230000002, 3.0009473029999998, 120.63115879999999, 355.37072819999997, 3.0012168840000002,
      104.91928830000001, 446.56621439999998, -2.4291254979999999, 119.3823773, -81.5035065, 90.071761339999995,
      120.84644969999999, 0.20142214319999999, 90.574182809999996, 113.35627599999999, 89.122484139999997,
      91.498786789999997, 128.82387059999999, 175.58544979999999, 101.4990564, 122.9002965, 268.38340210000001,
      91.499325959999993, 120.5156942, 355.88567010000003, 91.499595540000001, 131.39708440000001, 444.7709438,
      96.842661910000004, 119.9692496, -89.055928379999997, 182.5025977, 120.1122323, 0.20142214319999999, 179.9968959,
      122.4469038, 89.122484139999997, 179.9971654, 122.601725, 178.04354609999999, 179.997435, 126.95405100000001,
      266.96460810000002, 179.99770459999999, 109.6421946, 355.88567010000003, 180.75639219999999, 121.4688462,
      447.20440389999999, 185.2565166, 129.8153983, -89.080491530000003, 271.57375159999998, 118.9826401,
      0.20142214319999999, 268.49527449999999, 133.51210159999999, 89.122484139999997, 268.49554410000002, 123.2495995,
      178.04354609999999, 268.49581369999999, 156.55418539999999, 266.96460810000002, 268.49608330000001, 126.9632696,
      355.88567010000003, 268.49635280000001, 125.8377127, 456.11810259999999, 263.93876160000002, 124.1798611,
      -78.757516609999996, 354.1474331, 118.3106315, 0.20142214319999999, 356.99365319999998, 106.9986686,
      80.916068390000007, 356.99392280000001, 122.5179942, 183.90640450000001, 356.99419230000001, 139.70791149999999,
      274.36699199999998, 356.99446189999998, 128.8031953, 355.88567010000003, 356.9947315, 113.7361619, 431.15941249999997,
      357.78121429999999, 120.79886980000001, -87.137463229999994, 444.211366, 121.661636, 3.032267085, 442.4181438,
      122.02193889999999, 90.122988419999999, 447.11456579999998, 119.2880096, 171.88924359999999, 443.27612269999997,
      126.6859994, 266.08858839999999, 445.49284060000002, 123.3604062, 354.6575727, 447.03908790000003, 121.0512234,
      444.93700189999998, 444.38999630000001, 120.7991143, -88.437080820000006, -85.679078360000005, 248.01994110000001,
      0.90716082620000005, -87.579235199999999, 247.72265619999999, 90.422524789999997, -89.779116000000002,
      247.09381070000001, 177.9337969, -99.358937710000006, 246.43002150000001, 268.200943, -91.2776566, 246.78031390000001,
      356.61125870000001, -87.247415489999995, 247.33492340000001, 445.13719680000003, -85.752559039999994,
      247.14701299999999, -88.667440450000001, 2.378526865, 248.15868649999999, 2.8663826349999999, -1.1642714359999999,
      247.05720909999999, 89.347263699999999, -9.1457145759999996, 242.8314393, 174.87415490000001, -9.2338554550000005,
      240.81855210000001, 271.94218269999999, -10.368706850000001, 243.2065522, 357.89138079999998, -12.21101354,
      243.7249606, 445.92261459999997, 1.966463385, 247.059834, -88.912398969999998, 91.495463610000002, 248.26963259999999,
      2.7270593399999998, 91.117068270000004, 244.86417639999999, 82.594282609999993, 92.770709539999999,
      245.50448940000001, 178.86315379999999, 126.0907351, 229.5875126, 276.54213440000001, 100.338989, 241.38238050000001,
      357.5755231, 92.732282940000005, 243.25368760000001, 448.57513119999999, 91.815159890000004, 244.83041610000001,
      -90.320214309999997, 180.10600199999999, 246.54689540000001, 3.6745516139999999, 186.6898391, 241.52917769999999,
      89.349529239999995, 179.11642739999999, 239.24101239999999, 173.81978810000001, 162.44730430000001,
      250.41458320000001, 265.41958579999999, 180.45522170000001, 247.98236069999999, 353.23377349999998,
      192.21066049999999, 251.7287273, 445.66010940000001, 181.26521289999999, 246.71072000000001, -86.986800479999999,
      269.28480500000001, 244.58283109999999, 0.51393186719999995, 268.80303909999998, 236.98863900000001, 89.621769709999995,
      266.26399020000002, 238.93264049999999, 179.266944, 238.34859410000001, 265.1629236, 269.35367380000002, 256.8740277,
      257.8996176, 361.80705540000002, 263.43252030000002, 246.61417800000001, 436.91912619999999, 269.42811289999997,
      247.08582329999999, -87.15915038, 359.0044408, 247.66274319999999, -0.82839372629999997, 360.31312320000001,
      241.9673334, 92.841495690000002, 355.37873960000002, 250.24562259999999, 175.9934432, 348.99376549999999,
      257.76013139999998, 266.66541599999999, 356.44961849999999, 252.0638975, 353.65442949999999, 358.05145049999999,
      245.86233609999999, 442.59605629999999, 357.38704439999998, 247.363855, -88.558712749999998, 445.28969480000001,
      248.99420019999999, 0.055054464290000001, 448.6656423, 248.6825187, 89.390803320000003, 450.3772573,
      249.60217069999999, 177.06173150000001, 446.93556389999998, 250.2954866, 266.63590900000003, 444.83473020000002,
      249.33391330000001, 355.91280690000002, 445.81271129999999, 248.44589859999999, 444.89687249999997, 445.23544709999999 };

  cmtk::CoordinateVector::SmartPtr vParameters( new cmtk::CoordinateVector( sizeof( nparameters ), parameters, false /*free*/ ) );
  cmtk::SplineWarpXform splineWarp( cmtk::FixedVector<3,cmtk::Types::Coordinate>( domain ), cmtk::SplineWarpXform::IndexType( dims ), vParameters );

  int failed = 0, total = 0;
#pragma omp parallel for reduction(+:failed) reduction(+:total)
  for ( int k = 0; k < (int)domain[2]; k += 4 )
    {
    cmtk::Xform::SpaceVectorType v, vX, u;
    v[2] = k;
    for ( int j = 0; j < (int)domain[1]; j += 8 )
      {
      v[1] = j;
      for ( int i = 0; i < (int)domain[0]; i += 8 )
	{
	v[0] = i;

	++total;
	vX = splineWarp.Apply( v );
	const bool success = splineWarp.ApplyInverse( vX, u, 0.1 /*accuracy*/ );
	
	if ( ! success )
	  {
	  ++failed;
	  }
	}
      }
    }

  std::cerr << "Inversion failed for " << failed << " out of " << total << " points." << std::endl;

  return failed != 0;
}

