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


/// \file StereoSessionRPC.h
///

#ifndef __STEREO_SESSION_RPC_H__
#define __STEREO_SESSION_RPC_H__

#include <asp/Sessions/Pinhole/StereoSessionPinhole.h>

namespace asp {

  class StereoSessionRPC : public StereoSessionPinhole {

  public:
    virtual ~StereoSessionRPC() {}

    // Produces a camera model from the images
    virtual boost::shared_ptr<vw::camera::CameraModel>
    camera_model( std::string const& image_file,
                  std::string const& camera_file = "" ) {
      vw::vw_throw( vw::NoImplErr() << "StereoSessionRPC: Not implemented" );
    }

    static StereoSession* construct() { return new StereoSessionRPC; }
  };

}

#endif // __STEREO_SESSION_RPC_H__
