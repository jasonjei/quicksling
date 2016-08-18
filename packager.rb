require 'zlib'
require 'inifile'

sections = [ "resources", "support", "app" ]
products ||= "/Users/jjhung/Projects/quicklet-workspace/nginx/download/"
url = "http://download.quicklet.dev/"
files = {}
paths = {}
current_directory = File.expand_path(File.dirname(__FILE__))

paths["resources"] = "#{current_directory}/cef_binary/Resources/"
files["resources"] = [
"cef.pak",
"cef_100_percent.pak",
"cef_200_percent.pak",
"cef_extensions.pak",
"devtools_resources.pak",
"icudtl.dat",
"locales/am.pak",
"locales/ar.pak",
"locales/bg.pak",
"locales/bn.pak",
"locales/ca.pak",
"locales/cs.pak",
"locales/da.pak",
"locales/de.pak",
"locales/el.pak",
"locales/en-GB.pak",
"locales/en-US.pak",
"locales/es-419.pak",
"locales/es.pak",
"locales/et.pak",
"locales/fa.pak",
"locales/fi.pak",
"locales/fil.pak",
"locales/fr.pak",
"locales/gu.pak",
"locales/he.pak",
"locales/hi.pak",
"locales/hr.pak",
"locales/hu.pak",
"locales/id.pak",
"locales/it.pak",
"locales/ja.pak",
"locales/kn.pak",
"locales/ko.pak",
"locales/lt.pak",
"locales/lv.pak",
"locales/ml.pak",
"locales/mr.pak",
"locales/ms.pak",
"locales/nb.pak",
"locales/nl.pak",
"locales/pl.pak",
"locales/pt-BR.pak",
"locales/pt-PT.pak",
"locales/ro.pak",
"locales/ru.pak",
"locales/sk.pak",
"locales/sl.pak",
"locales/sr.pak",
"locales/sv.pak",
"locales/sw.pak",
"locales/ta.pak",
"locales/te.pak",
"locales/th.pak",
"locales/tr.pak",
"locales/uk.pak",
"locales/vi.pak",
"locales/zh-CN.pak",
"locales/zh-TW.pak"
]

# Dir.glob('#{current_directory}/cef_binary/Debug/**/*').select{ |e| File.file? e }.map { |v| v.gsub("#{current_directory}/cef_binary/Debug/", "") }
paths["support"] = "#{current_directory}/cef_binary/Release/"
files["support"] = [
"d3dcompiler_43.dll", 
"d3dcompiler_47.dll", 
"libcef.dll", 
"libEGL.dll", 
"libGLESv2.dll", 
"natives_blob.bin", 
"snapshot_blob.bin", 
"widevinecdmadapter.dll", 
"wow_helper.exe"
]

paths["bugsplat"] = "#{current_directory}/BugSplat/bin/"
files["bugsplat"] = [
  "BsSndRpt.exe",
  "BugSplat.dll",
  "BugSplatRc.dll"
]

paths["app"] = "#{current_directory}/Release/"
files["app"] = [
  "Quicksling.exe"
]

crcs = {}
files.keys.each do |key|
  crcs[key] = {}
  files[key].each do |file|
    crcs[key][file] = Zlib::crc32(File.read("#{paths[key]}#{file}")).to_s(16)
  end

  puts "cd #{paths[key]}; rm #{products}#{key}.zip; zip -r #{products}#{key}.zip #{files[key].join(" ")}"
  result = `cd #{paths[key]}; rm #{products}#{key}.zip; zip -r #{products}#{key}.zip #{files[key].join(" ")}`
  puts result
end


i = IniFile.new

files.keys.each do |key|
  files[key].each do |file|
    i[key][file] = Zlib::crc32(File.read("#{paths[key]}#{file}")).to_s(16)
  end
end

files.keys.each do |key|
  i['downloads'][key] = "#{url}#{key}.zip"
  i['checksums'][key] = Zlib::crc32(File.read("#{products}#{key}.zip")).to_s(16) 
end

File.write("#{products}download.ini", i.to_s)
