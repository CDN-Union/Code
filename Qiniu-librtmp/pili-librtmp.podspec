#
# Be sure to run `pod lib lint pili-librtmp.podspec' to ensure this is a
# valid spec and remove all comments before submitting the spec.
#
# Any lines starting with a # are optional, but encouraged
#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = "pili-librtmp"
  s.version          = "1.0.6"
  s.summary          = "pili-librtmp is a RTMP client library."
  s.homepage         = "https://github.com/pili-engineering/pili-librtmp"
  s.license          = 'LGPL'
  s.author           = { "pili" => "pili@qiniu.com" }
  s.source           = { :git => "https://github.com/pili-engineering/pili-librtmp.git", :tag => "v#{s.version}" }

  s.platform     = :ios, '7.0'
  s.requires_arc = true
  s.source_files = "Pod/Classes/pili-librtmp/*.{h,c}"
  s.exclude_files = "Pod/Classes/pili-librtmp/dh.h", "Pod/Classes/pili-librtmp/handshake.h"
end
