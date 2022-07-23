EntityContainer = {}



EntityContainer[ "Director" ] = {
    on_start = function( self )
        background2d:autoscroll_to_entity( entity_manager:get_entity( "Cloud" ) )
        script:request( Script.ENTITY, "Director", "main", 6 )
        return 0
    end,
    
    main = function( self )
        return 0
    end,
}



EntityContainer[ "Cloud" ] = {
    on_start = function( self )
        set_entity_to_character( "Cloud", "Cloud" )
        self.cloud = entity_manager:get_entity( "Cloud" )
        entity_manager:set_player_entity( "Cloud" )


        self.cloud:set_solid_radius( 0.3 )
        self.cloud:set_solid( true )

        return 0
    end,

    on_collide = function( self )
        return 0
    end,
}