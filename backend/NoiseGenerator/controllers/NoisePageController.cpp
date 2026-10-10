#include "NoisePageController.hpp"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(BYTE c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_encode(BYTE* buf, unsigned int bufLen) {
	std::string ret;
	int i = 0;
	int j = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];

	while (bufLen--) {
		char_array_3[i++] = *(buf++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

NoisePageController::NoisePageController()
{
	m_images_paths = std::filesystem::path(TEMP_FOLDER) / ".gen_images\\";
}

void NoisePageController::generate(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback,
                                   NOISEGENERATOR_NAMSEPACE::NoiseData &&noise_data) const
{
	NOISEGENERATOR_NAMSEPACE::NoiseRender render = 
		NOISEGENERATOR_NAMSEPACE::NoiseRender(noise_data.t, noise_data);
	auto image = render.createImage();
	
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 9999);

	std::ostringstream oss;
	oss << m_images_paths.string() << timestamp << "-" << std::setfill('0') << std::setw(4) << dis(gen) << ".bmp";

	std::filesystem::create_directories(std::filesystem::path(oss.str()).parent_path());

	noise::utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename(oss.str());
	writer.WriteDestFile();

	/*std::fstream file(oss.str());
	std::string bmp_data;
	file >> bmp_data;*/

	//FILE* fileptr;
	//BYTE* buffer;
	//long filelen;

	//fileptr = fopen(oss.str().c_str(), "rb");  // Open the file in binary mode
	//fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	//filelen = ftell(fileptr);             // Get the current byte offset in the file
	//rewind(fileptr);                      // Jump back to the beginning of the file

	//buffer = (BYTE*)malloc(filelen * sizeof(BYTE)); // Enough memory for the file
	//fread(buffer, 1, filelen, fileptr); // Read in the entire file
	//fclose(fileptr); // Close the file

	//std::string data = base64_encode(buffer, filelen);
	
	
	auto resp = drogon::HttpResponse::newFileResponse(oss.str(),
		"", drogon::CT_IMAGE_BMP, "", req);

	resp->setStatusCode(drogon::k200OK);
	/*resp->setContentTypeCode(drogon::CT_IMAGE_BMP);
	resp->setBody(bmp_data);*/
	callback(resp);
}

void NoisePageController::noise(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const
{
	auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "noise");
	auto resp = drogon::HttpResponse::newHttpViewResponse("NoisePageView", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void NoisePageController::download(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback)
{
	//req->setPath("/");

	
}
