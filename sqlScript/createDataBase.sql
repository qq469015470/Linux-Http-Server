SET @@autocommit=0;
START TRANSACTION;

CREATE DATABASE IF NOT EXISTS dangkou_prod;

USE dangkou_prod;

CREATE TABLE IF NOT EXISTS `wareHouse` 
(
	`id` varchar(36) NOT NULL,
	`name` varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
	PRIMARY KEY(`id`),
	UNIQUE KEY(`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE IF NOT EXISTS `material`
(
	`id` varchar(36) NOT NULL,
	`name` nvarchar(200) NOT NULL,
	PRIMARY KEY(`id`),
	UNIQUE KEY(`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE IF NOT EXISTS `itemInventory` 
(
	`id` varchar(36) NOT NULL,
	`wareHouseId` varchar(36) NOT NULL,
	`materialId` varchar(36) NOT NULL,
	`cost` decimal(10, 2) NOT NULL,
	`stock` decimal(10, 2) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE KEY (`id`),
	KEY `wareHouseId` (`wareHouseId`),
	KEY `materialId` (`materialId`),
	CONSTRAINT `itemInventory_ibfk_1` FOREIGN KEY (`wareHouseId`) REFERENCES `wareHouse` (`id`),
	CONSTRAINT `itemInventory_ibfk_2` FOREIGN KEY (`materialId`) REFERENCES `material` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE IF NOT EXISTS `checkIn`
(
	`id` varchar(36) NOT NULL,
	`itemInventoryId` varchar(36) NOT NULL,
	`number` decimal(10, 2) NOT NULL,
	`time` datetime NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE KEY (`id`),
	KEY `itemInventoryId` (`itemInventoryId`),
	CONSTRAINT `checkIn_ibfk_1` FOREIGN KEY (`itemInventoryId`) REFERENCES `itemInventory` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE IF NOT EXISTS `checkOut`
(
	`id` varchar(36) NOT NULL,
	`itemInventoryId` varchar(36) NOT NULL,
	`number` decimal(10, 2) NOT NULL,
	`time` datetime NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE KEY (`id`),
	KEY `itemInventoryId` (`itemInventoryId`),
	CONSTRAINT `checkOut_ibfk_1` FOREIGN KEY (`itemInventoryId`) REFERENCES `itemInventory` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

COMMIT;
SET @@autocommit=1; 
