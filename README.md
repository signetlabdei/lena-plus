# LENA+
This is a version of the LTE-EPC Network simulAtor extended with a realistic RACH model. It is based on the ns-3 LTE module and backward compatible with it. 

## Install
In order to use the realistic RACH model replace the content of src/lte folder of ns-3 with this repository.
The realistic RACH model must be activated by adding 

	Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
	Config::SetDefault ("ns3::LteHelper::UseIdealPrach", BooleanValue (false));  

to your scenario script. 
Moreover up to now only PfFfMacScheduler is supported, therefore add also 

	lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");

## Results
A paper with some experimental results obtained using the realistic RACH model was accepted for presentation at the IEEE ICC 2016 conference, May 23 -27, 2016, Kuala Lumpur, Malaysia, and can be found on arXiv: http://arxiv.org/abs/1601.05098
