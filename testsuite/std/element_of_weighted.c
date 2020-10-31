/**
 * NAME: element_of_weighted
 * SYNOPSIS: mixed element_of_weighted( mapping data )
 * DESCRIPTION: Returns a property from a mapping weighted by its value. 
 * The higher the weight (value), the more likely it will be returned.
 * EXAMPLE: element_of_weighted( ([ "element1" : 10, "element2" : 5 ]) )
 * Return value will more likely be "element1" due to its higher weight.
 * DEPENDENCIES: sum
 */

mixed element_of_weighted(mapping data)
{
    int roll_result, next_position, weight, max_weight;
    mixed *elems = ({ }), elem;
    int *weights = ({ }), *indices = ({ }), index = 0;

    foreach(elem, weight in data)
    {
        elems   = ({ elems..., elem });
        weights = ({ weights..., weight });
        indices = ({ indices..., index++ });
    }

    indices       = shuffle(indices);
    next_position = 0;
    max_weight    = OVERRIDES_FILE->sum(weights...);
    roll_result   = random(max_weight);

    foreach(index in indices)
    {
        //get that element's weight
        weight = weights[index];

        //our next_position is the point at which our next element
        //will be available. 
        next_position += weight;

        //if our random number (roll_result) falls before our next_position
        //then that's our guy.
        if(roll_result < next_position) return elems[index];
    }
}
