#ifndef READ_FILE_HPP
#define READ_FILE_HPP

namespace lib {
namespace fs = std::filesystem;

// auto read_file(const fs::path t_path) -> TextBuffer
// {
//   using std::filesystem::exists;

//   if(!exists(t_path)) {
//     std::stringstream ss{};

//     ss << "File does not exist! ";
//     ss << std::quoted(t_path.string());

//     throw std::invalid_argument{ss.str()};
//   }

//   TextBuffer text_buffer{t_path.string()};

//   std::ifstream ifs{t_path};
//   while(ifs.good() && !ifs.eof()) {
//     std::string line{};
//     std::getline(ifs, line);

//     text_buffer.add_line(std::move(line));
//   }

//   return text_buffer;
// }

} // namespace lib

#endif // READ_FILE_HPP
