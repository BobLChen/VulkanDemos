#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

enum ImageFilterType
{
    FilterNormal = 0,
    Filter3x3Convolution,
	FilterBilateralBlur,
	FilterBrightness,
	FilterBulgeDistortion,
	FilterCGAColorspace,
	FilterColorBalance,
	FilterColorInvert,
	FilterColorMatrix,
	FilterContrast,
	FilterCrosshatch,
	FilterDirectionalSobelEdgeDetection,
	FilterExposure,
	FilterFalseColor,
	FilterGamma,
	FilterGlassSphere,
	FilterGrayscale,
	FilterHalftone,
	FilterHaze,
	FilterHighlightShadow,
	FilterHue,
	FilterKuwahara,
	FilterLevels,
	FilterLuminance,
	FilterLuminanceThreshold,
	FilterMonochrome,
	FilterPixelation,
	FilterPosterize,
	FilterSharpen,
	FilterSolarize,
	FilterSphereRefraction,
    FilterCount
};

class RenderTargetDemo : public DemoBase
{
public:
	RenderTargetDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~RenderTargetDemo()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
		DemoBase::Setup();
		DemoBase::Prepare();

		InitParmas();
		CreateRenderTarget();
		CreateGUI();
		LoadAssets();
		
		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyRenderTarget();
		DestroyAssets();
		DestroyGUI();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct FilterItem
	{
		vk_demo::DVKMaterial*	material;
		vk_demo::DVKShader*		shader;
		ImageFilterType			type;

		void Create(const char* vert, const char* frag, std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, vk_demo::DVKTexture* rtt)
		{
			shader = vk_demo::DVKShader::Create(
				vulkanDevice,
				true,
				vert,
				frag
			);
			material = vk_demo::DVKMaterial::Create(
				vulkanDevice,
				renderPass,
				pipelineCache,
				shader
			);
			material->PreparePipeline();
			material->SetTexture("inputImageTexture", rtt);
		}

		void Destroy()
		{
			delete material;
			delete shader;
		}
	};

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct Filter3x3ConvolutionParamBlock
	{
		float		texelWidth;
		float		texelHeight;
        float       lineSize = 1;
		float		padding0;
		float		convolutionMatrix[16] = {
            -0.1f,  1.0f,  1.0f, 0.0f,
            -0.15f, 0.0f, -0.6f, 0.0f, 
            -0.2f,  0.0f, -1.0f, 0.0f,
			 0.0f,  0.0f,  0.0f, 0.0f
        };
	} filter3x3ConvolutionParam;

	struct FilterBilateralBlurParamBlock
	{
		Vector2  singleStepOffset;
		float	 distanceNormalizationFactor;
		float	 padding0;
	} filterBilateralBlurParam;

	struct FilterBrightnessParamBlock
	{
		float   brightness;
		Vector3 padding;
	} filterBrightnessParam;
	
	struct FilterBulgeDistortionParamBlock
	{
		float	aspectRatio;
		float	radius;
		float	scale;
		float	padding;
		Vector4 center;
	} filterBulgeDistortionParam;

	struct FilterColorBalanceParamBlock
	{
		Vector4 shadowsShift;
		Vector4 midtonesShift;
		Vector4 highlightsShift;
		int		preserveLuminosity;
		Vector3 padding;
	} filterColorBalanceParam;

	struct FilterColorMatrixParamBlock
	{
		Matrix4x4 colorMatrix;
		float     intensity;
		Vector3   padding;
	} filterColorMatrixParam;

	struct FilterContrastParamBlock
	{
		float	contrast;
		Vector3 padding;
	} filterContrastParam;

	struct FilterCrosshatchParamBlock
	{
		float crossHatchSpacing;
		float lineWidth;
		float padding1;
		float padding2;
	} filterCrosshatchParam;

	struct FilterDirectionalSobelEdgeDetectionParamBlock
	{
		float  texelWidth;
		float  texelHeight;
		float  lineSize;
		float  padding;
	} filterDirectionalSobelEdgeDetectionParam;

	struct FilterExposureParamBlock
	{
		float   exposure;
		Vector3 padding;
	} filterExposureParam;

	struct FilterFalseColorParamBlock
	{
		Vector4 firstColor;
		Vector4 secondColor;
	} filterFalseColorParam;

	struct FilterGammaParamBlock
	{
		Vector4 gamma;
	} filterGammaParam;

	struct FilterGlassSphereParamBlock
	{
		Vector4 center;
		float	radius;
		float	aspectRatio;
		float	refractiveIndex;
		float	padding;
	} filterGlassSphereParam;

	struct FilterHalftoneParamBlock
	{
		float fractionalWidthOfPixel;
		float aspectRatio;
		float padding0;
		float padding1;
	} filterHalftoneParam;

	struct FilterHazeParamBlock
	{
		float distance;
		float slope;
		float padding0;
		float padding1;
	} filterHazeParam;

	struct FilterHighlightShadowParamBlock
	{
		float	shadows;
		float	highlights;
		Vector2 padding;
	} filterHighlightShadowParam;
	
	struct FilterHueParamBlock
	{
		float   hueAdjust;
		float   hue;
		Vector2 padding;
	} filterHueParam;

	struct FilterKuwaharaParamBlock
	{
		int   radius;
		float padding0;
		float padding1;
		float padding2;
	} filterKuwaharaParam;

	struct FilterLevelsParamBlock
	{
		Vector4 levelMinimum;
		Vector4 levelMiddle;
		Vector4 levelMaximum;
		Vector4 minOutput;
		Vector4 maxOutput;
	} filterLevelsParam;

	struct FilterLuminanceThresholdParamBlock
	{
		float	threshold;
		Vector3 padding;
	} filterLuminanceThresholdParam;

	struct FilterMonochromeParamBlock
	{
		Vector3 filterColor;
		float   intensity;
	} filterMonochromeParam;

	struct FilterPixelationParamBlock
	{
		float imageWidthFactor;
		float imageHeightFactor;
		float pixel;
		float padding;
	} filterPixelationParam;

	struct FilterPosterizeParamBlock
	{
		float colorLevels;
		Vector3 padding;
	} filterPosterizeParam;

	struct FilterSharpenParamBlock
	{
		float imageWidthFactor;
		float imageHeightFactor;
		float sharpness;
		float padding2;
	} filterSharpenParam;

	struct FilterSolarizeParamBlock
	{
		float threshold;
		float padding0;
		float padding1;
		float padding2;
	} filterSolarizeParam;

	struct FilterSphereRefractionParamBlock
	{
		Vector4 center;
		float	radius;
		float	aspectRatio;
		float	refractiveIndex;
		float	padding;
	} filterSphereRefractionParam;

	struct FrameBufferObject
	{
		int32					width = 0;
		int32					height = 0;

		VkDevice				device = VK_NULL_HANDLE;
		VkFramebuffer			frameBuffer = VK_NULL_HANDLE;
		VkRenderPass			renderPass = VK_NULL_HANDLE;

		vk_demo::DVKTexture*	color = nullptr;
		vk_demo::DVKTexture*	depth = nullptr;

		void Destroy()
		{
			if (color) {
				delete color;
				color = nullptr;
			}

			if (depth) {
				delete depth;
				depth = nullptr;
			}

			if (frameBuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device, frameBuffer, VULKAN_CPU_ALLOCATOR);
				frameBuffer = VK_NULL_HANDLE;
			}

			if (renderPass != VK_NULL_HANDLE) {
				vkDestroyRenderPass(device, renderPass, VULKAN_CPU_ALLOCATOR);
				renderPass = VK_NULL_HANDLE;
			}
		}
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		// 设置Room参数
		m_ModelScene->rootNode->localMatrix.AppendRotation(delta * 90.0f, Vector3::UpVector);
		for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
		{
			m_SceneMaterials[i]->BeginFrame();
			for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
				m_MVPData.model = m_SceneMatMeshes[i][j]->linkNode->GetGlobalMatrix();
				m_SceneMaterials[i]->BeginObject();
				m_SceneMaterials[i]->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
				m_SceneMaterials[i]->EndObject();
			}
			m_SceneMaterials[i]->EndFrame();
		}

        UpdateFilterParams(time, delta);

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}
    
    void UpdateFilterParams(float time, float delta)
    {
		void*  data = nullptr;
		uint32 size = 0;
        switch (m_Selected) {
            case ImageFilterType::FilterNormal:
                // nothing
                break;
            case ImageFilterType::Filter3x3Convolution:
				data = &filter3x3ConvolutionParam;
				size = sizeof(Filter3x3ConvolutionParamBlock);
				break;
			case ImageFilterType::FilterBilateralBlur:
				data = &filterBilateralBlurParam;
				size = sizeof(FilterBilateralBlurParamBlock);
				break;
			case ImageFilterType::FilterBrightness:
				data = &filterBrightnessParam;
				size = sizeof(FilterBrightnessParamBlock);
				break;
			case ImageFilterType::FilterBulgeDistortion:
				data = &filterBulgeDistortionParam;
				size = sizeof(FilterBulgeDistortionParamBlock);
				break;
			case ImageFilterType::FilterColorBalance:
				data = &filterColorBalanceParam;
				size = sizeof(FilterColorBalanceParamBlock);
				break;
			case ImageFilterType::FilterColorMatrix:
				data = &filterColorMatrixParam;
				size = sizeof(FilterColorMatrixParamBlock);
				break;
			case ImageFilterType::FilterContrast:
				data = &filterContrastParam;
				size = sizeof(FilterContrastParamBlock);
				break;
			case ImageFilterType::FilterCrosshatch:
				data = &filterCrosshatchParam;
				size = sizeof(FilterCrosshatchParamBlock);
				break;
			case ImageFilterType::FilterDirectionalSobelEdgeDetection:
				data = &filterDirectionalSobelEdgeDetectionParam;
				size = sizeof(FilterDirectionalSobelEdgeDetectionParamBlock);
				break;
			case ImageFilterType::FilterExposure:
				data = &filterExposureParam;
				size = sizeof(FilterExposureParamBlock);
				break;
			case ImageFilterType::FilterFalseColor:
				data = &filterFalseColorParam;
				size = sizeof(FilterFalseColorParamBlock);
				break;
			case ImageFilterType::FilterGamma:
				data = &filterGammaParam;
				size = sizeof(FilterGammaParamBlock);
				break;
			case ImageFilterType::FilterGlassSphere:
				data = &filterGlassSphereParam;
				size = sizeof(FilterGlassSphereParamBlock);
				break;
			case ImageFilterType::FilterHalftone:
				data = &filterHalftoneParam;
				size = sizeof(FilterHalftoneParamBlock);
				break;
			case ImageFilterType::FilterHaze:
				data = &filterHazeParam;
				size = sizeof(FilterHazeParamBlock);
				break;
			case ImageFilterType::FilterHighlightShadow:
				data = &filterHighlightShadowParam;
				size = sizeof(FilterHighlightShadowParamBlock);
				break;
			case ImageFilterType::FilterHue:
				data = &filterHueParam;
				size = sizeof(FilterHueParamBlock);
				break;
			case ImageFilterType::FilterKuwahara:
				data = &filterKuwaharaParam;
				size = sizeof(FilterKuwaharaParamBlock);
				break;
			case ImageFilterType::FilterLevels:
				data = &filterLevelsParam;
				size = sizeof(FilterLevelsParamBlock);
				break;
			case ImageFilterType::FilterLuminanceThreshold:
				data = &filterLuminanceThresholdParam;
				size = sizeof(FilterLuminanceThresholdParamBlock);
				break;
			case ImageFilterType::FilterMonochrome:
				data = &filterMonochromeParam;
				size = sizeof(FilterMonochromeParamBlock);
				break;
			case ImageFilterType::FilterPixelation:
				data = &filterPixelationParam;
				size = sizeof(FilterPixelationParamBlock);
				break;
			case ImageFilterType::FilterPosterize:
				data = &filterPosterizeParam;
				size = sizeof(FilterPosterizeParamBlock);
				break;
			case ImageFilterType::FilterSharpen:
				data = &filterSharpenParam;
				size = sizeof(FilterSharpenParamBlock);
				break;
			case ImageFilterType::FilterSolarize:
				data = &filterSolarizeParam;
				size = sizeof(FilterSolarizeParamBlock);
				break;
			case ImageFilterType::FilterSphereRefraction:
				data = &filterSphereRefractionParam;
				size = sizeof(FilterSphereRefractionParamBlock);
				break;
            default:
                break;
        }

		if (data == nullptr || size == 0) {
			return;
		}

		vk_demo::DVKMaterial* material = m_FilterItems[m_Selected].material;
		material->BeginFrame();
		material->BeginObject();
		material->SetLocalUniform("filterParam", data, size);
		material->EndObject();
		material->EndFrame();
    }
	
    void UpdateFilter3x3ConvolutionUI()
    {
        ImGui::SliderFloat("LineSize", &filter3x3ConvolutionParam.lineSize, 0.1f, 10.0f);
        filter3x3ConvolutionParam.texelWidth  = filter3x3ConvolutionParam.lineSize / m_FrameWidth;
        filter3x3ConvolutionParam.texelHeight = filter3x3ConvolutionParam.lineSize / m_FrameHeight;
        
        ImGui::SliderFloat3("Row0", (filter3x3ConvolutionParam.convolutionMatrix + 0), -1.0f, 1.0f);
        ImGui::SliderFloat3("Row1", (filter3x3ConvolutionParam.convolutionMatrix + 4), -1.0f, 1.0f);
        ImGui::SliderFloat3("Row2", (filter3x3ConvolutionParam.convolutionMatrix + 8), -1.0f, 1.0f);
    }

	void UpdateFilterBilateralBlurUI()
	{
		ImGui::SliderFloat("Factor", &filterBilateralBlurParam.distanceNormalizationFactor, 0.0f, 5.0f);
	}

	void UpdateFilterBrightnessUI()
	{
		ImGui::SliderFloat("Brightness", &filterBrightnessParam.brightness, 0.0f, 5.0f);
	}
    
	void UpdateFilterBulgeDistortionUI()
	{
		ImGui::SliderFloat("Radius", &filterBulgeDistortionParam.radius, 0.0f, 5.0f);
		ImGui::SliderFloat("Scale", &filterBulgeDistortionParam.scale,  0.0f, 5.0f);
		ImGui::SliderFloat2("Center", (float*)&filterBulgeDistortionParam.center, 0.0f, 1.0f);
	}

	void UpdateFilterColorBalanceUI()
	{
		ImGui::SliderFloat3("Showdows", (float*)&filterColorBalanceParam.shadowsShift, 0.0f, 5.0f);
		ImGui::SliderFloat3("Midtones", (float*)&filterColorBalanceParam.midtonesShift, 0.0f, 5.0f);
		ImGui::SliderFloat3("Highlights", (float*)&filterColorBalanceParam.highlightsShift, 0.0f, 5.0f);
		bool checked = filterColorBalanceParam.preserveLuminosity > 0 ? true : false;
		ImGui::Checkbox("PreserveLuminosity", &checked);
		filterColorBalanceParam.preserveLuminosity = checked ? 1 : 0;
	}

	void UpdateFilterColorMatrixUI()
	{
		ImGui::SliderFloat4("Row0", filterColorMatrixParam.colorMatrix.m[0], 0.0f, 1.0f);
		ImGui::SliderFloat4("Row1", filterColorMatrixParam.colorMatrix.m[1], 0.0f, 1.0f);
		ImGui::SliderFloat4("Row2", filterColorMatrixParam.colorMatrix.m[2], 0.0f, 1.0f);
		ImGui::SliderFloat4("Row3", filterColorMatrixParam.colorMatrix.m[3], 0.0f, 1.0f);
		ImGui::SliderFloat("Intensity", &filterColorMatrixParam.intensity, 0.0f, 5.0f);
	}

	void UpdateFilterContrastUI()
	{
		ImGui::SliderFloat("Contrast", &filterContrastParam.contrast, 0.0f, 4.0f);
	}

	void UpdateFilterCrosshatchUI()
	{
		ImGui::SliderFloat("CrossHatchSpacing", &filterCrosshatchParam.crossHatchSpacing, 0.0f, 1.0f);
		ImGui::SliderFloat("LineWidth", &filterCrosshatchParam.lineWidth, 0.0f, 0.01f);
	}

	void UpdateFilterDirectionalSobelEdgeDetectionUI()
	{
		ImGui::SliderFloat("LineSize", &filterDirectionalSobelEdgeDetectionParam.lineSize, 0.0f, 1.0f);
		filterDirectionalSobelEdgeDetectionParam.texelWidth  = filterDirectionalSobelEdgeDetectionParam.lineSize / m_FrameWidth;
		filterDirectionalSobelEdgeDetectionParam.texelHeight = filterDirectionalSobelEdgeDetectionParam.lineSize / m_FrameHeight;
	}

	void UpdateFilterExposureUI()
	{
		ImGui::SliderFloat("Exposure", &filterExposureParam.exposure, -10.0f, 10.0f);
	}

	void UpdateFilterFalseColorUI()
	{
		ImGui::ColorEdit3("FirstColor", (float*)&filterFalseColorParam.firstColor);
		ImGui::ColorEdit3("SecondColor", (float*)&filterFalseColorParam.secondColor);
	}

	void UpdateFilterGammaUI()
	{
		ImGui::SliderFloat3("Gamma", (float*)&filterGammaParam.gamma, 0.0f, 10.0f);
	}

	void UpdateFilterGlassSphereUI()
	{
		ImGui::SliderFloat2("Center", (float*)&filterGlassSphereParam.center, 0.0f, 1.0f);
		ImGui::SliderFloat("Radius", &filterGlassSphereParam.radius, 0.0f, 5.0f);
		ImGui::SliderFloat("RefractiveIndex", &filterGlassSphereParam.refractiveIndex, 0.0f, 1.0f);
	}

	void UpdateFilterHalftoneUI()
	{
		ImGui::SliderFloat("Fractional", &filterHalftoneParam.fractionalWidthOfPixel, 0.0f, 0.10f);
	}

	void UpdateFilterHazeUI()
	{
		ImGui::SliderFloat("Distance", &filterHazeParam.distance, 0.0f, 1.0f);
		ImGui::SliderFloat("Slope", &filterHazeParam.slope, 0.0f, 1.0f);
	}

	void UpdateFilterHighlightShadowUI()
	{
		ImGui::SliderFloat("Shadows", &filterHighlightShadowParam.shadows, 0.0f, 1.0f);
		ImGui::SliderFloat("Highlights", &filterHighlightShadowParam.highlights, 0.0f, 1.0f);
	}

	void UpdateFilterHueUI()
	{
		ImGui::SliderFloat("Hue", &filterHueParam.hue, 0.0f, 360.0f);
		filterHueParam.hueAdjust = filterHueParam.hue * PI / 180.0f;
	}

	void UpdateFilterKuwaharaUI()
	{
		ImGui::SliderInt("Radius", &filterKuwaharaParam.radius, 0, 30);
	}

	void UpdateFilterLevelsUI()
	{
		ImGui::SliderFloat3("LevelMinimum", (float*)&filterLevelsParam.levelMinimum, 0.0f, 1.0f);
		ImGui::SliderFloat3("LevelMiddle", (float*)&filterLevelsParam.levelMiddle, 0.0f, 1.0f);
		ImGui::SliderFloat3("LevelMaximum", (float*)&filterLevelsParam.levelMaximum, 0.0f, 1.0f);
		ImGui::SliderFloat3("MinOutput", (float*)&filterLevelsParam.minOutput, 0.0f, 1.0f);
		ImGui::SliderFloat3("MaxOutput", (float*)&filterLevelsParam.maxOutput, 0.0f, 1.0f);
	}

	void UpdateFilterLuminanceThresholdUI()
	{
		ImGui::SliderFloat("Threshold", &filterLuminanceThresholdParam.threshold, 0.0f, 1.0f);
	}

	void UpdateFilterMonochromeUI()
	{
		ImGui::ColorEdit3("FilterColor", (float*)&filterMonochromeParam.filterColor);
		ImGui::SliderFloat("Intensity", &filterMonochromeParam.intensity, 0.0f, 10.0f);
	}

	void UpdateFilterPixelationUI()
	{
		ImGui::SliderFloat("Pixel", &filterPixelationParam.pixel, 0.0f, 10.0f);
	}

	void UpdateFilterPosterizeUI()
	{
		ImGui::SliderFloat("ColorLevels", &filterPosterizeParam.colorLevels, 1.0f, 256.0f);
	}

	void UpdateFilterSharpenUI()
	{
		ImGui::SliderFloat("Sharpness", &filterSharpenParam.sharpness, -4.0f, 4.0f);
	}

	void UpdateFilterSolarizeUI()
	{
		ImGui::SliderFloat("Threshold", &filterSolarizeParam.threshold, 0.0f, 1.0f);
	}

	void UpdateFilterSphereRefractionUI()
	{
		ImGui::SliderFloat2("Center", (float*)&filterSphereRefractionParam.center, 0.0f, 1.0f);
		ImGui::SliderFloat2("Radius", &filterSphereRefractionParam.radius, 0.0f, 1.0f);
		ImGui::SliderFloat("Refractive", &filterSphereRefractionParam.refractiveIndex, 0.0f, 1.0f);
	}

    void UpdateFilterUI(float time, float delta)
    {
        switch (m_Selected) {
            case ImageFilterType::FilterNormal:
                // nothing
                break;
            case ImageFilterType::Filter3x3Convolution:
                UpdateFilter3x3ConvolutionUI();
				break;
			case ImageFilterType::FilterBilateralBlur:
				UpdateFilterBilateralBlurUI();
				break;
			case ImageFilterType::FilterBrightness:
				UpdateFilterBrightnessUI();
				break;
			case ImageFilterType::FilterBulgeDistortion:
				UpdateFilterBulgeDistortionUI();
				break;
			case ImageFilterType::FilterColorBalance:
				UpdateFilterColorBalanceUI();
				break;
			case ImageFilterType::FilterColorMatrix:
				UpdateFilterColorMatrixUI();
				break;
			case ImageFilterType::FilterContrast:
				UpdateFilterContrastUI();
				break;
			case ImageFilterType::FilterCrosshatch:
				UpdateFilterCrosshatchUI();
				break;
			case ImageFilterType::FilterDirectionalSobelEdgeDetection:
				UpdateFilterDirectionalSobelEdgeDetectionUI();
				break;
			case ImageFilterType::FilterExposure:
				UpdateFilterExposureUI();
				break;
			case ImageFilterType::FilterFalseColor:
				UpdateFilterFalseColorUI();
				break;
			case ImageFilterType::FilterGamma:
				UpdateFilterGammaUI();
				break;
			case ImageFilterType::FilterGlassSphere:
				UpdateFilterGlassSphereUI();
				break;
			case ImageFilterType::FilterHalftone:
				UpdateFilterHalftoneUI();
				break;
			case ImageFilterType::FilterHaze:
				UpdateFilterHazeUI();
				break;
			case ImageFilterType::FilterHighlightShadow:
				UpdateFilterHighlightShadowUI();
				break;
			case ImageFilterType::FilterHue:
				UpdateFilterHueUI();
				break;
			case ImageFilterType::FilterKuwahara:
				UpdateFilterKuwaharaUI();
				break;
			case ImageFilterType::FilterLevels:
				UpdateFilterLevelsUI();
				break;
			case ImageFilterType::FilterLuminanceThreshold:
				UpdateFilterLuminanceThresholdUI();
				break;
			case ImageFilterType::FilterMonochrome:
				UpdateFilterMonochromeUI();
				break;
			case ImageFilterType::FilterPixelation:
				UpdateFilterPixelationUI();
				break;
			case ImageFilterType::FilterPosterize:
				UpdateFilterPosterizeUI();
				break;
			case ImageFilterType::FilterSharpen:
				UpdateFilterSharpenUI();
				break;
			case ImageFilterType::FilterSolarize:
				UpdateFilterSolarizeUI();
				break;
			case ImageFilterType::FilterSphereRefraction:
				UpdateFilterSphereRefractionUI();
				break;
            default:
                break;
        }
    }
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("RenderTargetDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			
            ImGui::Combo("Filter", &m_Selected, m_FilterNames.data(), m_FilterNames.size());
            
            UpdateFilterUI(time, delta);
            
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateRenderTarget()
	{
		m_RenderTarget.device = m_Device;
		m_RenderTarget.width  = m_FrameWidth;
		m_RenderTarget.height = m_FrameHeight;

		m_RenderTarget.color = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		m_RenderTarget.depth = vk_demo::DVKTexture::CreateRenderTarget(
            m_VulkanDevice,
            PixelFormatToVkFormat(m_DepthFormat, false),
            VK_IMAGE_ASPECT_DEPTH_BIT,
            m_FrameWidth, m_FrameHeight,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        
		std::vector<VkAttachmentDescription> attchmentDescriptions(2);
		// Color attachment
		attchmentDescriptions[0].format         = m_RenderTarget.color->format;
		attchmentDescriptions[0].samples        = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format         = m_RenderTarget.depth->format;
		attchmentDescriptions[1].samples        = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference;
		depthReference.attachment = 1;
		depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount    = 1;
		subpassDescription.pColorAttachments       = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		std::vector<VkSubpassDependency> dependencies(2);
		dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass      = 0;
		dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass      = 0;
		dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create renderpass
		VkRenderPassCreateInfo renderPassInfo;
		ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
		renderPassInfo.attachmentCount = attchmentDescriptions.size();
		renderPassInfo.pAttachments    = attchmentDescriptions.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpassDescription;
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies   = dependencies.data();
		VERIFYVULKANRESULT(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &(m_RenderTarget.renderPass)));
		
		VkImageView attachments[2];
		attachments[0] = m_RenderTarget.color->imageView;
		attachments[1] = m_RenderTarget.depth->imageView;

		VkFramebufferCreateInfo frameBufferInfo;
		ZeroVulkanStruct(frameBufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferInfo.renderPass      = m_RenderTarget.renderPass;
		frameBufferInfo.attachmentCount = 2;
		frameBufferInfo.pAttachments    = attachments;
		frameBufferInfo.width           = m_RenderTarget.width;
		frameBufferInfo.height          = m_RenderTarget.height;
		frameBufferInfo.layers          = 1;
		VERIFYVULKANRESULT(vkCreateFramebuffer(m_Device, &frameBufferInfo, VULKAN_CPU_ALLOCATOR, &(m_RenderTarget.frameBuffer)));
	}

	void DestroyRenderTarget()
	{
		m_RenderTarget.Destroy();
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;
		
		// room model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
		);
		// room shader
		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/22_RenderTarget/obj.vert.spv",
			"assets/shaders/22_RenderTarget/obj.frag.spv"
		);
		// Room textures
		std::vector<std::string> diffusePaths = {
			"assets/models/Room/miniHouse_Part1.jpg",
			"assets/models/Room/miniHouse_Part2.jpg",
			"assets/models/Room/miniHouse_Part3.jpg",
			"assets/models/Room/miniHouse_Part4.jpg"
		};
		m_SceneDiffuses.resize(diffusePaths.size());
		for (int32 i = 0; i < diffusePaths.size(); ++i)
		{
			m_SceneDiffuses[i] = vk_demo::DVKTexture::Create2D(
				diffusePaths[i],
				m_VulkanDevice,
				cmdBuffer
			);
		}
		// room material
		m_SceneMaterials.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_SceneMaterials.size(); ++i)
		{
			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RenderPass,
				m_PipelineCache,
				m_SceneShader
			);
			m_SceneMaterials[i]->PreparePipeline();
			m_SceneMaterials[i]->SetTexture("diffuseMap", m_SceneDiffuses[i]);
		}
		// collect meshles
		m_SceneMatMeshes.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_ModelScene->meshes.size(); ++i)
		{
			vk_demo::DVKMesh* mesh = m_ModelScene->meshes[i];
			const std::string& diffuseName = mesh->material.diffuse;
			if (diffuseName == "miniHouse_Part1") {
				m_SceneMatMeshes[0].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part2") {
				m_SceneMatMeshes[1].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part3") {
				m_SceneMatMeshes[2].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part4") {
				m_SceneMatMeshes[3].push_back(mesh);
			}
		}

		delete cmdBuffer;

		// ------------------------- Filters -------------------------
		m_FilterNames.resize(ImageFilterType::FilterCount);
		m_FilterItems.resize(ImageFilterType::FilterCount);
		m_FilterSpirvs.resize(ImageFilterType::FilterCount * 2);

#define DefineFilter(FilterType, FilterName) \
        m_FilterNames[FilterType] = FilterName; \
        m_FilterSpirvs[FilterType * 2 + 0] = "assets/shaders/22_RenderTarget/" FilterName ".vert.spv"; \
        m_FilterSpirvs[FilterType * 2 + 1] = "assets/shaders/22_RenderTarget/" FilterName ".frag.spv"; \
        
		DefineFilter(ImageFilterType::FilterNormal,							"Normal");
		DefineFilter(ImageFilterType::Filter3x3Convolution,					"Filter3x3Convolution");
		DefineFilter(ImageFilterType::FilterBilateralBlur,					"FilterBilateralBlur");
		DefineFilter(ImageFilterType::FilterBrightness,						"FilterBrightness");
		DefineFilter(ImageFilterType::FilterBulgeDistortion,				"FilterBulgeDistortion");
		DefineFilter(ImageFilterType::FilterCGAColorspace,					"FilterCGAColorspace");
		DefineFilter(ImageFilterType::FilterColorBalance,					"FilterColorBalance");
		DefineFilter(ImageFilterType::FilterColorInvert,					"FilterColorInvert");
		DefineFilter(ImageFilterType::FilterColorMatrix,					"FilterColorMatrix");
		DefineFilter(ImageFilterType::FilterContrast,						"FilterContrast");
		DefineFilter(ImageFilterType::FilterCrosshatch,						"FilterCrosshatch");
		DefineFilter(ImageFilterType::FilterDirectionalSobelEdgeDetection,	"FilterDirectionalSobelEdgeDetection");
		DefineFilter(ImageFilterType::FilterExposure,						"FilterExposure");
		DefineFilter(ImageFilterType::FilterFalseColor,						"FilterFalseColor");
		DefineFilter(ImageFilterType::FilterGamma,							"FilterGamma");
		DefineFilter(ImageFilterType::FilterGlassSphere,					"FilterGlassSphere");
		DefineFilter(ImageFilterType::FilterGrayscale,						"FilterGrayscale");
		DefineFilter(ImageFilterType::FilterHalftone,						"FilterHalftone");
		DefineFilter(ImageFilterType::FilterHaze,							"FilterHaze");
		DefineFilter(ImageFilterType::FilterHighlightShadow,				"FilterHighlightShadow");
		DefineFilter(ImageFilterType::FilterHue,							"FilterHue");
		DefineFilter(ImageFilterType::FilterKuwahara,						"FilterKuwahara");
		DefineFilter(ImageFilterType::FilterLevels,							"FilterLevels");
		DefineFilter(ImageFilterType::FilterLuminance,						"FilterLuminance");
		DefineFilter(ImageFilterType::FilterLuminanceThreshold,				"FilterLuminanceThreshold");
		DefineFilter(ImageFilterType::FilterMonochrome,						"FilterMonochrome");
		DefineFilter(ImageFilterType::FilterPixelation,						"FilterPixelation");
		DefineFilter(ImageFilterType::FilterPosterize,						"FilterPosterize");
		DefineFilter(ImageFilterType::FilterSharpen,						"FilterSharpen");
		DefineFilter(ImageFilterType::FilterSolarize,						"FilterSolarize");
		DefineFilter(ImageFilterType::FilterSphereRefraction,				"FilterSphereRefraction");
		
#undef DefineFilter

		// 创建Filter
		for (int32 i = 0; i < ImageFilterType::FilterCount; ++i)
		{
			m_FilterItems[i].Create(
				m_FilterSpirvs[i * 2 + 0],
				m_FilterSpirvs[i * 2 + 1],
				m_VulkanDevice,
				m_RenderPass,
				m_PipelineCache,
				m_RenderTarget.color
			);
		}
		
		m_Selected = 0;
	}
    
	void DestroyAssets()
	{
		delete m_SceneShader;

		delete m_ModelScene;

		for (int32 i = 0; i < m_SceneDiffuses.size(); ++i) {
			delete m_SceneDiffuses[i];
		}
		m_SceneDiffuses.clear();

		for (int32 i = 0; i < m_SceneMaterials.size(); ++i) {
			delete m_SceneMaterials[i];
		}
		m_SceneMaterials.clear();

		for (int32 i = 0; i < ImageFilterType::FilterCount; ++i) {
			m_FilterItems[i].Destroy();
		}
		m_FilterItems.clear();
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = m_FrameHeight;
		viewport.width    = m_FrameWidth;
		viewport.height   = -(float)m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// render target pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderTarget.renderPass;
			renderPassBeginInfo.framebuffer              = m_RenderTarget.frameBuffer;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_RenderTarget.width;
			renderPassBeginInfo.renderArea.extent.height = m_RenderTarget.height;
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterials[i]->GetPipeline());
				for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
					m_SceneMaterials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
					m_SceneMatMeshes[i][j]->BindDrawCmd(commandBuffer);
				}
			}

			vkCmdEndRenderPass(commandBuffer);
		}

		// second pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderPass;
			renderPassBeginInfo.framebuffer              = m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);
            
			{
				vk_demo::DVKMaterial* material = m_FilterItems[m_Selected].material;
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetPipeline());
				material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}
			
			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 10.0f, 5000.0f);
		m_ViewCamera.SetPosition(0, 500.0f, -1500.0f);
		m_ViewCamera.LookAt(0, 0, 0);

		{
            filter3x3ConvolutionParam.lineSize    = 1.0f;
			filter3x3ConvolutionParam.texelWidth  = 1.0f / m_FrameWidth;
			filter3x3ConvolutionParam.texelHeight = 1.0f / m_FrameHeight;
		}

		{
			filterBilateralBlurParam.singleStepOffset.x = 1.0f / m_FrameWidth;
			filterBilateralBlurParam.singleStepOffset.y = 1.0f / m_FrameHeight;
			filterBilateralBlurParam.distanceNormalizationFactor = 0.8f;
		}

		{
			filterBrightnessParam.brightness = 0.0f;
		}

		{
			filterBulgeDistortionParam.aspectRatio = (float)m_FrameHeight / m_FrameWidth;
			filterBulgeDistortionParam.radius      = 0.25f;
			filterBulgeDistortionParam.scale       = 0.5f;
			filterBulgeDistortionParam.center.x    = 0.5f;
			filterBulgeDistortionParam.center.y    = 0.5f;
		}

		{
			filterColorBalanceParam.shadowsShift.Set(0.5f, 1.0f, 0, 0);
			filterColorBalanceParam.midtonesShift.Set(0.5f, 0, 0, 0);
			filterColorBalanceParam.highlightsShift.Set(0.5f, 0, 0, 0);
			filterColorBalanceParam.preserveLuminosity = 1;
		}

		{
			filterColorMatrixParam.colorMatrix.CopyRawFrom(0, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
			filterColorMatrixParam.colorMatrix.CopyRawFrom(1, Vector4(0.0f, 1.0f, 1.0f, 0.0f));
			filterColorMatrixParam.colorMatrix.CopyRawFrom(2, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			filterColorMatrixParam.colorMatrix.CopyRawFrom(3, Vector4(0.0f, 0.0f, 1.0f, 0.0f));
			filterColorMatrixParam.intensity = 1.0f;
		}

		{
			filterContrastParam.contrast = 4.0f;
		}

		{
			filterCrosshatchParam.crossHatchSpacing = 0.012f;
			filterCrosshatchParam.lineWidth = 0.006f;
		}

		{
			filterDirectionalSobelEdgeDetectionParam.lineSize    = 1.0f;
			filterDirectionalSobelEdgeDetectionParam.texelWidth  = 1.0f / m_FrameWidth;
			filterDirectionalSobelEdgeDetectionParam.texelHeight = 1.0f / m_FrameHeight;
		}

		{
			filterExposureParam.exposure = 0.5f;
		}

		{
			filterFalseColorParam.firstColor.Set(0.0f, 0.0f, 0.5f, 0.0f);
			filterFalseColorParam.secondColor.Set(1.0f, 0.0f, 0.0f, 0.0f);
		}

		{
			filterGammaParam.gamma.Set(2.2f, 2.2f, 2.2f, 0.0f);
		}

		{
			filterGlassSphereParam.center.Set(0.5f, 0.5f, 0.0f, 0.0f);
			filterGlassSphereParam.radius = 0.25f;
			filterGlassSphereParam.refractiveIndex = 0.71f;
			filterGlassSphereParam.aspectRatio = (float)m_FrameHeight / m_FrameWidth;
		}

		{
			filterHalftoneParam.fractionalWidthOfPixel = 0.01f;
			filterHalftoneParam.aspectRatio = (float)m_FrameHeight / m_FrameWidth;
		}

		{
			filterHazeParam.distance = 0.7f;
			filterHazeParam.slope = 0.0f;
		}

		{
			filterHighlightShadowParam.shadows = 0.25f;
			filterHighlightShadowParam.highlights = 1.0f;
		}

		{
			filterHueParam.hue = 100.0f;
			filterHueParam.hueAdjust = 100.0f * PI / 180.0f;
		}

		{
			filterKuwaharaParam.radius = 15;
		}

		{
			filterLevelsParam.levelMinimum.Set(0.5f, 0.5f, 0.5f, 1.0f);
			filterLevelsParam.levelMiddle.Set(1.0f, 1.0f, 1.0f, 1.0f);
			filterLevelsParam.levelMaximum.Set(1.0f, 1.0f, 1.0f, 1.0f);
			filterLevelsParam.minOutput.Set(0.0f, 0.25f, 0.0f, 1.0f);
			filterLevelsParam.maxOutput.Set(1.0f, 1.0f, 1.0f, 1.0f);
		}

		{
			filterLuminanceThresholdParam.threshold = 0.5f;
		}

		{
			filterMonochromeParam.filterColor.Set(0.6f, 0.45f, 0.3f);
			filterMonochromeParam.intensity = 2.0f;
		}

		{
			filterPixelationParam.imageWidthFactor = 1.0f / m_FrameWidth;
			filterPixelationParam.imageHeightFactor = 1.0f / m_FrameHeight;
			filterPixelationParam.pixel = 5.0f;
		}

		{
			filterPosterizeParam.colorLevels = 1.0f;
		}

		{
			filterSharpenParam.imageWidthFactor = 1.0f / m_FrameWidth;
			filterSharpenParam.imageHeightFactor = 1.0f / m_FrameHeight;
			filterSharpenParam.sharpness = 2.0f;
		}

		{
			filterSolarizeParam.threshold = 0.5f;
		}

		{
			filterSphereRefractionParam.center.Set(0.5f, 0.5f, 0.0f, 0.0f);
			filterSphereRefractionParam.radius = 0.25f;
			filterSphereRefractionParam.refractiveIndex = 0.71f;
			filterSphereRefractionParam.aspectRatio = (float)m_FrameHeight / m_FrameWidth;
		}
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Ubuntu-Regular.ttf");
	}
	
	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:

	typedef std::vector<vk_demo::DVKTexture*>			TextureArray;
	typedef std::vector<vk_demo::DVKMaterial*>			MaterialArray;
	typedef std::vector<std::vector<vk_demo::DVKMesh*>> MatMeshArray;

	bool 						m_Ready = false;

	vk_demo::DVKCamera			m_ViewCamera;

	FrameBufferObject			m_RenderTarget;

	vk_demo::DVKModel*			m_Quad = nullptr;

	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_ModelScene = nullptr;
	vk_demo::DVKShader*			m_SceneShader = nullptr;
	TextureArray				m_SceneDiffuses;
	MaterialArray				m_SceneMaterials;
	MatMeshArray				m_SceneMatMeshes;

    std::vector<const char*>    m_FilterNames;
	std::vector<const char*>	m_FilterSpirvs;
	std::vector<FilterItem>		m_FilterItems;
    int32                       m_Selected = 0;
    
	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<RenderTargetDemo>(1400, 900, "RenderTargetDemo", cmdLine);
}
