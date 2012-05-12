// __BEGIN_LICENSE__
//  Copyright (c) 2009-2012, United States Government as represented by the
//  Administrator of the National Aeronautics and Space Administration. All
//  rights reserved.
//
//  The NGT platform is licensed under the Apache License, Version 2.0 (the
//  "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// __END_LICENSE__


#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <vw/FileIO.h>
#include <vw/Image.h>
#include <vw/Cartography.h>
#include <vw/Math.h>

using std::cout;
using std::endl;
using std::string;

using namespace vw;
using namespace vw::cartography;

int main( int argc, char *argv[] ) {
  string dem1_name, dem2_name, output_prefix;
  double default_value;

  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "Display this help message")
    ("default-value", po::value(&default_value), "The value of missing pixels in the first dem")
    ("output-prefix,o", po::value(&output_prefix), "Specify the output prefix");

  po::options_description positional("");
  positional.add_options()
    ("dem1", po::value(&dem1_name), "Explicitly specify the first dem")
    ("dem2", po::value(&dem2_name), "Explicitly specify the second dem");

  po::positional_options_description p;
  p.add("dem1", 1);
  p.add("dem2", 1);

  po::options_description all_options;
  all_options.add(desc).add(positional);

  po::variables_map vm;
  try {
    po::store( po::command_line_parser( argc, argv ).options(desc).positional(p).run(), vm );
    po::notify( vm );
  } catch (po::error const& e) {
    cout << "Error parsing: " << e.what() << "\n\t" << desc << "\n";
    return 1;
  }

  if ( dem1_name.empty() || dem2_name.empty() || vm.count("help" ) ) {
    cout << "Usage: " << argv[0] << " <dem1> <dem2> " << endl;
    cout << desc << endl;
    return 1;
  }

  if ( output_prefix.empty() ) {
    fs::path dem1_path(dem1_name), dem2_path(dem2_name);
    output_prefix = (dem1_path.branch_path() / (fs::basename(dem1_path) + "__" + fs::basename(dem2_path))).string();
  }

  DiskImageResourceGDAL dem1_rsrc(dem1_name), dem2_rsrc(dem2_name);
  if ( !vm.count("default-value") && dem1_rsrc.has_nodata_read() ) {
    default_value = dem1_rsrc.nodata_read();
    vw_out() << "\tFound input nodata value: " << default_value << endl;
  }
  DiskImageView<double> dem1_dmg(dem1_name), dem2_dmg(dem2_name);

  GeoReference dem1_georef, dem2_georef;
  read_georeference(dem1_georef, dem1_rsrc);
  read_georeference(dem2_georef, dem2_rsrc);

  // dem2_reproj is calculated in the event that two dem's datums are 
  // different (for example, USGS uses 3396190m for the radius of Mars, 
  // while we use 3396000m)
  ImageViewRef<double> dem2_reproj = select_channel(reproject_point_image(dem_to_point_image(dem2_dmg, dem2_georef), dem2_georef, dem1_georef), 2);
  ImageViewRef<double> dem2_trans = crop(geo_transform(dem2_reproj, dem2_georef, dem1_georef), 0, 0, dem1_dmg.cols(), dem1_dmg.rows());

  ImageViewRef<PixelMask<double> > diff_masked = copy_mask(dem1_dmg - dem2_trans, create_mask(dem1_dmg, default_value));
  ImageViewRef<double> diff = apply_mask(diff_masked, default_value);

  DiskImageResourceGDAL output_rsrc(output_prefix + "-diff.tif", diff.format(), 
                                         Vector2i(vw_settings().default_tile_size(), 
                                                  vw_settings().default_tile_size()));
  write_georeference(output_rsrc, dem1_georef);
  block_write_image(output_rsrc, diff,
                    TerminalProgressCallback("asp", "\t--> Differencing: "));

  return 0;
}
