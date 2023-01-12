#include "layer.hpp"
#include "layer.hpp"

namespace ve
{
	LayerStack::~LayerStack()
	{
		for (Layer* layer : layers)
		{
			layer->onDetach();
			delete layer;
		}
	}
	/**
	* @brief Pushes a layer to the layer stack
	*/
	void LayerStack::pushLayer(Layer* layer)
	{
		layers.emplace(layers.begin() + layerInsertIndex, layer); 
		layerInsertIndex++;
	}
	
	/**
	* @brief Push an overlay to the top of the stack. Overlays are always rendered on top of layers.
	*/
	void LayerStack::pushOverlay(Layer* overlay)
	{
		layers.emplace_back(overlay);
	}

	void LayerStack::popLayer(Layer* layer)
	{
		auto it = std::find(layers.begin(), layers.end(), layer);
		if (it != layers.end())
		{
			layers.erase(it);
			layerInsertIndex--;
		}
	}
	
	void LayerStack::popOverlay(Layer* overlay)
	{
		auto it = std::find(layers.begin(), layers.end(), overlay);
		if (it != layers.end())
		{
			layers.erase(it);
		}
	}
}