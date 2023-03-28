#include "Core/Layer.hpp"

namespace Nyxis
{
	LayerStack::~LayerStack()
	{
		for (Layer* layer : layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}
	/**
	* @brief Pushes a layer to the layer stack
	*/
	void LayerStack::PushLayer(Layer* layer)
	{
		layers.emplace(layers.begin() + layerInsertIndex, layer); 
		layerInsertIndex++;
	}
	
	/**
	* @brief Push an overlay to the top of the stack. Overlays are always rendered on top of layers.
	*/
	void LayerStack::PushOverlay(Layer* overlay)
	{
		layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(layers.begin(), layers.end(), layer);
		if (it != layers.end())
		{
			layers.erase(it);
			layerInsertIndex--;
		}
	}
	
	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(layers.begin(), layers.end(), overlay);
		if (it != layers.end())
		{
			layers.erase(it);
		}
	}
}